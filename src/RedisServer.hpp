#pragma once

#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <memory>
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

class KeyValueStore;

class RedisServer
{
  public:
    RedisServer(int port);
    ~RedisServer();
    void start();

  private:
    int server_fd;
    int port;
    std::vector<std::thread> workerThreads;
    std::unique_ptr<KeyValueStore> keyValueStore;

    bool createServerSocket();
    bool bindServer();
    bool listenForConnections();
    void acceptConnections();

    void handleConnection(int client_fd);
    void onPing(int client_fd);
    void onEcho(int client_fd, const std::vector<std::string>& command);
    void onSet(int client_fd, const std::vector<std::string>& command);
    void onGet(int client_fd, const std::vector<std::string>& command);

    std::optional<std::string> readFromSocket(int client_fd);
};