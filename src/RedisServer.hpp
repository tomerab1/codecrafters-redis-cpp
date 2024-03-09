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

class KeyValueStore;
class CommandDispatcher;

class RedisServer
{
  public:
    RedisServer(int port);
    ~RedisServer();
    void start();

    inline KeyValueStore* getKVStore()
    {
        return keyValueStore.get();
    }

    inline void setRole(const std::string& role)
    {
        this->role = role;
    }

    inline std::string getRole()
    {
        return role;
    }

  private:
    int serverFd;
    int port;
    std::string role {"master"};
    std::vector<std::thread> workerThreads;
    std::unique_ptr<KeyValueStore> keyValueStore;
    std::unique_ptr<CommandDispatcher> cmdDispatcher;

    bool createServerSocket();
    bool bindServer();
    bool listenForConnections();
    void acceptConnections();
    void handleConnection(int clientFd);

    std::optional<std::string> readFromSocket(int client_fd);
};