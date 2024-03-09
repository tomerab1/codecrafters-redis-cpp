#include "RedisServer.hpp"

#include "CommandDispatcher.hpp"
#include "KeyValueStore.hpp"
#include "Parser.hpp"
#include "ResponseBuilder.hpp"

static constexpr int MAX_BUFFER = 4096;

RedisServer::RedisServer(int port) :
    port(port), keyValueStore {std::make_unique<KeyValueStore>()},
    cmdDispatcher {std::make_unique<CommandDispatcher>()}
{}

RedisServer::~RedisServer()
{
    for (auto&& thread : workerThreads)
    {
        thread.join();
    }
}

void RedisServer::start()
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

    acceptConnections();
}

bool RedisServer::createServerSocket()
{
    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    return (serverFd >= 0);
}

bool RedisServer::bindServer()
{
    int reuse = 1;
    if (setsockopt(serverFd,
                   SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT,
                   &reuse,
                   sizeof(reuse)) < 0)
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