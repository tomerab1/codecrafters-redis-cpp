#include "RedisServer.hpp"

#include "KeyValueStore.hpp"
#include "Parser.hpp"
#include "ResponseBuilder.hpp"

static constexpr std::string PING_STR = "ping";
static constexpr std::string SET_STR = "set";
static constexpr std::string GET_STR = "get";
static constexpr std::string ECHO_STR = "echo";
static constexpr std::string INFO_STR = "info";
static constexpr std::string PONG_STR = "+PONG\r\n";
static constexpr int MAX_BUFFER = 4096;

RedisServer::RedisServer(int port) :
    port(port), keyValueStore {std::make_unique<KeyValueStore>()}
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
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    return (server_fd >= 0);
}

bool RedisServer::bindServer()
{
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) <
        0)
    {
        std::cerr << "setsockopt failed\n";
        return false;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    return (bind(server_fd,
                 (struct sockaddr*)&server_addr,
                 sizeof(server_addr)) == 0);
}

bool RedisServer::listenForConnections()
{
    return (listen(server_fd, SOMAXCONN) == 0);
}

void RedisServer::acceptConnections()
{
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    std::cout << "Waiting for a client to connect...\n";

    while (true)
    {
        int client_fd =
            accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_fd < 0)
        {
            std::cerr << "Failed to accept\n";
        }
        else
        {
            char ipAddr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, ipAddr, INET_ADDRSTRLEN);
            std::cout << "New connection from: " << ipAddr << port << "\n";
            workerThreads.emplace_back(
                &RedisServer::handleConnection, this, client_fd);
        }
    }
}

void RedisServer::handleConnection(int client_fd)
{
    std::optional<std::string> buffer;
    while ((buffer = readFromSocket(client_fd)))
    {
        std::vector<std::string> parsedCommand = Parser::parseCommand(*buffer);
        if (parsedCommand.empty())
        {
            break;
        }

        if (parsedCommand[0] == ECHO_STR)
        {
            onEcho(client_fd, parsedCommand);
        }
        else if (parsedCommand[0] == PING_STR)
        {
            onPing(client_fd);
        }
        else if (parsedCommand[0] == SET_STR)
        {
            onSet(client_fd, parsedCommand);
        }
        else if (parsedCommand[0] == GET_STR)
        {
            onGet(client_fd, parsedCommand);
        }
        else if (parsedCommand[0] == INFO_STR)
        {
            onInfo(client_fd, parsedCommand);
        }
        else
        {
            onInvalidCommand(client_fd, parsedCommand);
        }
    }

    close(client_fd);
}

void RedisServer::onPing(int client_fd)
{
    if (send(client_fd, PONG_STR.data(), PONG_STR.length(), 0) < 0)
    {
        std::cerr << "Could not send PONG to client\n";
    }
}

void RedisServer::onEcho(int client_fd, const std::vector<std::string>& command)
{
    if (command.size() < 2)
    {
        onInvalidArgs(client_fd, command);
    }
    std::string response = ResponseBuilder::bulkString(command[1]);
    if (send(client_fd, response.data(), response.length(), 0) < 0)
    {
        std::cerr << "Could not send ECHO response to client\n";
    }
}

void RedisServer::onSet(int client_fd, const std::vector<std::string>& command)
{
    if (command.size() < 3)
    {
        onInvalidArgs(client_fd, command);
    }
    else
    {
        std::string response = "";
        if (std::find(command.begin(), command.end(), "px") == command.end())
        {
            response = keyValueStore->set(command[1], command[2]);
        }
        else
        {
            if (command.size() < 4)
            {
                onInvalidArgs(client_fd, command);
            }
            else
            {
                response =
                    keyValueStore->set(command[1], command[2], command[4]);
            }
        }
        if (send(client_fd, response.data(), response.length(), 0) < 0)
        {
            std::cerr << "Could not send SET response to client\n";
        }
    }
}

void RedisServer::onGet(int client_fd, const std::vector<std::string>& command)
{
    if (command.size() < 2)
    {
        onInvalidArgs(client_fd, command);
    }
    else
    {
        auto response = keyValueStore->get(command[1]);
        if (send(client_fd, response.data(), response.length(), 0) < 0)
        {
            std::cerr << "Could not send GET response to client\n";
        }
    }
}

void RedisServer::onInfo(int client_fd, const std::vector<std::string>& command)
{
    if (command.size() < 1)
    {
        onInvalidArgs(client_fd, command);
    }
    else
    {
        auto response = ResponseBuilder::bulkString("role:" + role);
        if (send(client_fd, response.data(), response.length(), 0) < 0)
        {
            std::cerr << "Could not send GET response to client\n";
        }
    }
}

void RedisServer::onInvalidArgs(int client_fd,
                                const std::vector<std::string>& command)
{
    auto response =
        ResponseBuilder::error("ERR wrong number of arguments for command");
    if (send(client_fd, response.data(), response.length(), 0) < 0)
    {
        std::cerr << "Could not send ERROR response to client\n";
    }
}

void RedisServer::onInvalidCommand(int client_fd,
                                   const std::vector<std::string>& command)
{
    auto response = ResponseBuilder::error("ERR \"" + command[0] +
                                           "\" is not valid command");
    if (send(client_fd, response.data(), response.length(), 0) < 0)
    {
        std::cerr << "Could not send ERROR response to client\n";
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