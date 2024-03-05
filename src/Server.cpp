#include "Parser.hpp"  // Assuming Parser.hpp contains the Parser class definition

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <optional>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

static constexpr std::string pingStr = "ping";
static constexpr std::string echoStr = "echo";
static constexpr std::string pongStr = "+PONG\r\n";
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

    bool createServerSocket()
    {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        return (server_fd >= 0);
    }

    bool bindServer()
    {
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

            if (parsedCommand[0] == echoStr)
            {
                onEcho(client_fd, parsedCommand);
            }
            else if (parsedCommand[0] == pingStr)
            {
                onPing(client_fd);
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
        if (send(client_fd, pongStr.data(), pongStr.length(), 0) < 0)
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
        std::string response = encodeBulk(command[1]);
        if (send(client_fd, response.data(), response.length(), 0) < 0)
        {
            std::cerr << "Could not send ECHO response to client\n";
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

    std::string encodeBulk(const std::string& str)
    {
        return "$" + std::to_string(str.length()) + "\r\n" + str + "\r\n";
    }
};

int main(int argc, char** argv)
{
    std::cout << "Logs from your program will appear here!\n";

    RedisServer server(6379);
    server.start();

    return 0;
}
