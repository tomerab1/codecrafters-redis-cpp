#pragma once
#include "KeyValueStore.hpp"
#include "Parser.hpp"

#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <format>
#include <iostream>
#include <netinet/in.h>
#include <optional>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

static constexpr std::string PING_STR = "ping";
static constexpr std::string SET_STR = "set";
static constexpr std::string GET_STR = "get";
static constexpr std::string ECHO_STR = "echo";
static constexpr std::string PONG_STR = "+PONG\r\n";
static constexpr int MAX_BUFFER = 4096;

class RedisServer
{
  public:
    RedisServer(int port) : port(port)
    {}

    ~RedisServer()
    {
        for (auto&& thread : workerThreads)
        {
            thread.join();
        }
    }

    void start()
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

  private:
    int server_fd;
    int port;
    std::vector<std::thread> workerThreads;
    KeyValueStore keyValueStore;

    bool createServerSocket()
    {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        return (server_fd >= 0);
    }

    bool bindServer()
    {
        int reuse = 1;
        if (setsockopt(
                server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0)
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

    bool listenForConnections()
    {
        return (listen(server_fd, SOMAXCONN) == 0);
    }

    void acceptConnections()
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        std::cout << "Waiting for a client to connect...\n";

        while (true)
        {
            int client_fd = accept(
                server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
            if (client_fd < 0)
            {
                std::cerr << "Failed to accept\n";
            }
            else
            {
                char ipAddr[INET_ADDRSTRLEN];
                inet_ntop(
                    AF_INET, &client_addr.sin_addr, ipAddr, INET_ADDRSTRLEN);
                std::cout << std::format(
                    "New connection from: {}:{}\n", ipAddr, port);
                workerThreads.emplace_back(
                    &RedisServer::handleConnection, this, client_fd);
            }
        }
    }

    void handleConnection(int client_fd)
    {
        std::optional<std::string> buffer;
        while ((buffer = readFromSocket(client_fd)))
        {
            std::vector<std::string> parsedCommand =
                Parser::parseCommand(*buffer);
            if (parsedCommand.empty())
            {
                break;
            }

            std::transform(parsedCommand[0].begin(),
                           parsedCommand[0].end(),
                           parsedCommand[0].begin(),
                           ::tolower);

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
            else
            {
                break;
            }
        }

        close(client_fd);
    }

    void onPing(int client_fd)
    {
        if (send(client_fd, PONG_STR.data(), PONG_STR.length(), 0) < 0)
        {
            std::cerr << "Could not send PONG to client\n";
        }
    }

    void onEcho(int client_fd, const std::vector<std::string>& command)
    {
        if (command.size() < 2)
        {
            return;
        }
        std::string response = ResponseBuilder::bulkString(command[1]);
        if (send(client_fd, response.data(), response.length(), 0) < 0)
        {
            std::cerr << "Could not send ECHO response to client\n";
        }
    }

    void onSet(int client_fd, const std::vector<std::string>& command)
    {
        if (command.size() < 3)
        {
            auto response = ResponseBuilder::error(
                "ERR wrong number of arguments for command");
            if (send(client_fd, response.data(), response.length(), 0) < 0)
            {
                std::cerr << "Could not send ERROR response to client\n";
            }
        }
        else
        {
            auto response = keyValueStore.set(command[1], command[2]);
            if (send(client_fd, response.data(), response.length(), 0) < 0)
            {
                std::cerr << "Could not send SET response to client\n";
            }
        }
    }

    void onGet(int client_fd, const std::vector<std::string>& command)
    {
        if (command.size() < 2)
        {
            auto response = ResponseBuilder::error(
                "ERR wrong number of arguments for command");
            if (send(client_fd, response.data(), response.length(), 0) < 0)
            {
                std::cerr << "Could not send ERROR response to client\n";
            }
        }
        else
        {
            auto response = keyValueStore.get(command[1]);
            if (send(client_fd, response.data(), response.length(), 0) < 0)
            {
                std::cerr << "Could not send GET response to client\n";
            }
        }
    }

    std::optional<std::string> readFromSocket(int client_fd)
    {
        char buffer[MAX_BUFFER];
        ssize_t numRecv = recv(client_fd, buffer, sizeof(buffer), 0);
        if (numRecv <= 0)
        {
            return std::nullopt;
        }
        return std::string(buffer, buffer + numRecv);
    }
};