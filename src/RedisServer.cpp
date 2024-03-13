#include "RedisServer.hpp"

#include "CommandDispatcher.hpp"
#include "KeyValueStore.hpp"
#include "Parser.hpp"
#include "Replication/ReplicationInfo.hpp"
#include "ResponseBuilder.hpp"

#include <algorithm>
#include <csignal>

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

    instancePointer = this;

    std::signal(SIGINT, handleSIGINT);
}

RedisServer::~RedisServer()
{
    for (auto&& thread : workerThreads)
    {
        thread.join();
    }
}

void RedisServer::handleSIGINT(int signal)
{
    std::cout << "Shuting down the server...\n";
    assert(instancePointer != nullptr);
    instancePointer->shutdown();
    exit(signal);
}

void RedisServer::shutdown()
{
    mShouldTerminateDistribution.store(true);
    if (replInfo->getRole() == "master")
    {
        for (auto clientFd : replInfo->getReplicaVector())
        {
            close(clientFd);
        }
        replInfo->getReplicaVector().clear();
    }
    else
    {
        close(masterFd);
    }

    close(serverFd);
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
        replInfo->setMasterFd(masterFd);
        performHandshakeWithMaster();
        startMasterConnectionHandlerThread();
    }
    catch (std::runtime_error& e)
    {
        throw e;
    }
}

void RedisServer::performHandshakeWithMaster()
{
    const std::vector<std::pair<std::string, std::vector<std::string> > >
        handshakeCommands = {
            {"ping", {}},
            {"replconf", {"listening-port", std::to_string(port)}},
            {"replconf", {"capa", "psync2"}},
            {"psync", {"?", "-1"}}};

    const std::unordered_map<std::string, std::string> handshakeExpectedRes = {
        {"ping", "+PONG"}, {"replconf", "+OK"}, {"psync", "+FULLRESYNC"}};

    std::optional<std::string> response;

    for (const auto& [commandName, args] : handshakeCommands)
    {
        sendCommandToMaster(commandName, args);
        response = readFromSocket(masterFd);

        if (response.has_value() &&
            response.value().find(handshakeExpectedRes.at(commandName)) ==
                std::string::npos)
        {
            std::cerr << response.value() << "\n";
        }
    }

    processReceivedCommands(*response);
}

void RedisServer::processReceivedCommands(const std::string& response)
{
    const auto commandPos = response.find("*");
    if (commandPos != std::string::npos)
    {
        const auto parsedCommands =
            Parser::parseCommand(response.substr(commandPos));

        for (const auto& parsedCommand : parsedCommands)
        {
            if (!parsedCommand.empty())
            {
                cmdDispatcher->dispatch(masterFd, parsedCommand, this);
            }
        }
    }
}

void RedisServer::startMasterConnectionHandlerThread()
{
    replInfo->setFinishedHandshake(true);
    workerThreads.emplace_back(&RedisServer::handleConnection, this, masterFd);
}

void RedisServer::sendCommandToMaster(const std::string& command,
                                      const std::vector<std::string>& args)
{
    std::vector<std::string> commandVector {command};
    commandVector.reserve(commandVector.size() + args.size() + 1);
    std::transform(args.begin(),
                   args.end(),
                   std::back_inserter(commandVector),
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
    reuse = 1;
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) <
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

    if (replInfo->getRole() == "master")
    {
        workerThreads.emplace_back(&RedisServer::distributeCommandsFromBuffer,
                                   this);
    }

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
            std::cout << "New connection from: " << ipAddr << ":" << port
                      << "\n";
            workerThreads.emplace_back(
                &RedisServer::handleConnection, this, clientFd);
        }
    }
}

void RedisServer::distributeCommandsFromBuffer()
{
    while (!mShouldTerminateDistribution)
    {
        {
            std::unique_lock<std::mutex> lock(commandBufferMtx);
            if (!commandBuffer.empty())
            {
                auto rawCommand = ResponseBuilder::array(commandBuffer.front());
                commandBuffer.pop_front();
                lock.unlock();
                distributeCommandToReplicas(rawCommand);
            }
        }
    }
}

void RedisServer::distributeCommandToReplicas(const std::string& rawCommand)
{
    for (auto client : replInfo->getReplicaVector())
    {
        if (send(client, rawCommand.c_str(), rawCommand.length(), 0) < 0)
        {
            std::cerr << "Could not send command to replica\n";
        }
    }
}

void RedisServer::handleConnection(int clientFd)
{
    bool shouldExit {false};
    std::optional<std::string> buffer;
    while ((buffer = readFromSocket(clientFd)) != std::nullopt && !shouldExit)
    {
        const auto commandPos = (*buffer).find("*");
        if (commandPos != std::string::npos)
        {
            auto parsedCommands =
                Parser::parseCommand((*buffer).substr(commandPos));

            if (parsedCommands.empty())
            {
                shouldExit = true;
            }
            for (auto parsedCommand : parsedCommands)
            {
                if (parsedCommand.empty())
                {
                    shouldExit = true;
                    break;
                }

                cmdDispatcher->dispatch(clientFd, parsedCommand, this);
            }
        }
        else
        {
            shouldExit = true;
        }
    }

    if (replInfo->getRole() == "master")
    {
        auto clientFdVector = replInfo->getReplicaVector();
        if (std::find(clientFdVector.begin(), clientFdVector.end(), clientFd) ==
            clientFdVector.end())
        {
            close(clientFd);
        }
    }
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