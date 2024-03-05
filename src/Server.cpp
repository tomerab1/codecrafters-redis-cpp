#include "Parser.hpp"

#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <optional>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

static constexpr std::string pingStr = "ping";
static constexpr std::string echoStr = "echo";
static constexpr std::string pongStr = "+PONG\r\n";
static constexpr int MAX_BUFFER = 4096;

class Buffer
{
  public:
    Buffer()
    {
        memset(buffer, 0, sizeof(buffer));
        readIdx = 0;
    }

    char* getBuffer()
    {
        return buffer;
    }

    std::string toString()
    {
        return std::string(buffer);
    }

    int size()
    {
        return MAX_BUFFER;
    }

  private:
    char buffer[MAX_BUFFER];
    int readIdx;
};

std::optional<Buffer> readSome(int client_fd)
{
    Buffer buff;

    int numRecv;
    if ((numRecv = recv(client_fd, buff.getBuffer(), buff.size(), 0)) < 0)
    {
        std::cout << "Error: " << std::strerror(errno) << "\n";
    }
    if (numRecv == 0)
    {
        return std::nullopt;
    }

    return buff;
}

std::string encodeBulk(const std::string& str)
{
    return "$" + std::to_string(str.length()) + "\r\n" + str + "\r\n";
}

void parseRedisString(const std::string& redisStr)
{}

void onPing(int client_fd)
{
    if (send(client_fd, pongStr.data(), pongStr.length(), 0) < 0)
    {
        std::cout << "Could not ping client\n";
    }
}

void onEcho(int client_fd, const std::string& toEcho)
{
    auto bulkString = encodeBulk(toEcho);

    if (send(client_fd, bulkString.data(), bulkString.length(), 0) < 0)
    {
        std::cout << "Could not ping client\n";
    }
}

void onConnection(int client_fd)
{
    std::optional<Buffer> buffer = std::nullopt;

    while ((buffer = readSome(client_fd)) != std::nullopt)
    {
        if (buffer == std::nullopt)
        {
            break;
        }

        auto parsedCommand = Parser::parseCommand(buffer.value().toString());
        if (parsedCommand.empty())
        {
            break;
        }

        if (parsedCommand[0] == echoStr)
        {
            onEcho(client_fd, parsedCommand[1]);
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

int main(int argc, char** argv)
{
    // You can use print statements as follows for debugging, they'll be visible
    // when running tests.
    std::cout << "Logs from your program will appear here!\n";

    // Uncomment this block to pass the first stage
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        std::cerr << "Failed to create server socket\n";
        return 1;
    }

    // // Since the tester restarts your program quite often, setting REUSE_PORT
    // // ensures that we don't run into 'Address already in use' errors
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) <
        0)
    {
        std::cerr << "setsockopt failed\n";
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(6379);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) !=
        0)
    {
        std::cerr << "Failed to bind to port 6379\n";
        return 1;
    }

    int connection_backlog = 5;
    if (listen(server_fd, connection_backlog) != 0)
    {
        std::cerr << "listen failed\n";
        return 1;
    }

    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    std::cout << "Waiting for a client to connect...\n";

    std::vector<std::thread> threads;
    while (true)
    {
        int client_fd = accept(server_fd,
                               (struct sockaddr*)&client_addr,
                               (socklen_t*)&client_addr_len);
        if (client_fd < 0)
        {
            std::cout << "Failed to accept\n";
        }
        else
        {
            threads.emplace_back(onConnection, client_fd);
        }
    }

    for (auto&& thread : threads)
    {
        thread.join();
    }

    close(server_fd);

    return 0;
}
