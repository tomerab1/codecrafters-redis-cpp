#include "RedisServer.hpp"

#include "CommandDispatcher.hpp"
#include "KeyValueStore.hpp"
#include "Parser.hpp"
#include "Replication/ReplicationInfo.hpp"
#include "ResponseBuilder.hpp"

#include <algorithm>

static constexpr int MAX_BUFFER = 4096;

RedisServer::RedisServer(int port, bool isMaster) :
    port(port), keyValueStore {std::make_unique<KeyValueStore>()},
    cmdDispatcher {std::make_unique<CommandDispatcher>()}
{
    if (isMaster)
    {
        replInfo = std::make_unique<ReplicationInfo>("master");
    }
    else
    {
        replInfo = std::make_unique<ReplicationInfo>("slave");
    }
}

RedisServer::~RedisServer()
{
    for (auto&& thread : workerThreads)
    {
        thread.join();
    }
}

void RedisServer::start(int masterPort, const std::string& masterAddr)
{
    if (!createServerSocket())
    {
        std::cerr << "Failed to create server socket\n";
        return;
    }

    if (!bindServer())
    {
        std::cerr << "Failed to bind to port " << port << "\n";
        return;
    }

    if (!listenForConnections())
    {
        std::cerr << "Failed to listen on port " << port << "\n";
        return;
    }

    if (replInfo->getRole() != "master")
    {
        handshake(masterPort, masterAddr);
    }

    acceptConnections();
}

void RedisServer::connectToMaster(int masterPort, const std::string& masterAddr)
{
    masterFd = socket(AF_INET, SOCK_STREAM, 0);
    if (masterFd < 0)
    {
        throw std::runtime_error("Error: Could not create socket");
    }

    struct sockaddr_in inMasterAddr;
    memset(&inMasterAddr, 0, sizeof(inMasterAddr));
    inMasterAddr.sin_family = AF_INET;
    inMasterAddr.sin_port = htons(masterPort);
    // sockAddr.sin_addr.s_addr = inet_addr(masterAddr.c_str());

    if (connect(masterFd,
                (struct sockaddr*)&inMasterAddr,
                sizeof(inMasterAddr)) < 0)
    {
        throw std::runtime_error(
            "Error: could not connect to server from replica");
    }
}

void RedisServer::handshake(int masterPort, const std::string& masterAddr)
{
    try
    {
        connectToMaster(masterPort, masterAddr);
        sendCommandToMaster("ping");

        auto response = readFromSocket(masterFd);
        if (response.has_value() &&
            response.value().find("PONG") != std::string::npos)
        {
            // Send REPLCONF (1st) to master
            sendCommandToMaster("replconf",
                                {"listening-port", std::to_string(port)});

            // Read OK response from master
            response = readFromSocket(masterFd);
            if (response.has_value() &&
                response.value().find("OK") != std::string::npos)
            {
                // Send REPLCONF (2nd) to master
                sendCommandToMaster("replconf", {"capa", "psync2"});
            }
        }
    }
    catch (std::runtime_error& e)
    {
        throw e;
    }
}

void RedisServer::sendCommandToMaster(const std::string& command,
                                      const std::vector<std::string>& args = {})
{
    std::vector<std::string> commandVector {command};
    std::transform(args.begin(),
                   args.end(),
                   std::back_inserter(args),
                   [](auto& arg) { return arg; });
    auto commandReq = ResponseBuilder::array(commandVector);

    if (send(masterFd, commandReq.c_str(), commandReq.length(), 0) < 0)
    {
        throw std::runtime_error("Could not send command to master");
    }
}

bool RedisServer::createServerSocket()
{
    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    return (serverFd >= 0);
}

bool RedisServer::bindServer()
{
    int reuse = 1;
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
        0)
    {
        std::cerr << "setsockopt failed\n";
        return false;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    return (bind(serverFd,
                 (struct sockaddr*)&server_addr,
                 sizeof(server_addr)) == 0);
}

bool RedisServer::listenForConnections()
{
    return (listen(serverFd, SOMAXCONN) == 0);
}

void RedisServer::acceptConnections()
{
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    std::cout << "Waiting for a client to connect...\n";

    while (true)
    {
        int clientFd =
            accept(serverFd, (struct sockaddr*)&client_addr, &client_addr_len);
        if (clientFd < 0)
        {
            std::cerr << "Failed to accept\n";
        }
        else
        {
            char ipAddr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, ipAddr, INET_ADDRSTRLEN);
            std::cout << "New connection from: " << ipAddr << port << "\n";
            workerThreads.emplace_back(
                &RedisServer::handleConnection, this, clientFd);
        }
    }
}

void RedisServer::handleConnection(int clientFd)
{
    std::optional<std::string> buffer;
    while ((buffer = readFromSocket(clientFd)))
    {
        std::vector<std::string> parsedCommand = Parser::parseCommand(*buffer);
        if (parsedCommand.empty())
        {
            break;
        }

        cmdDispatcher->dispatch(clientFd, parsedCommand, this);
    }

    close(clientFd);
}

std::optional<std::string> RedisServer::readFromSocket(int client_fd)
{
    char buffer[MAX_BUFFER];
    ssize_t numRecv = recv(client_fd, buffer, sizeof(buffer), 0);
    if (numRecv <= 0)
    {
        return std::nullopt;
    }
    return std::string(buffer, buffer + numRecv);
}