#pragma once

#include <algorithm>
#include <arpa/inet.h>
#include <cassert>
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
class ReplicationInfo;
class CommandDispatcher;

class RedisServer
{
  public:
    RedisServer(int port, bool isMaster);
    ~RedisServer();
    void start(int masterPort = 0, const std::string& masterAddr = "");
    void connectToMaster(int masterPort, const std::string& masterAddr);
    void handshake(int masterPort, const std::string& masterAddr);

    inline KeyValueStore* getKVStore()
    {
        assert(keyValueStore.get());
        return keyValueStore.get();
    }

    inline ReplicationInfo* getReplInfo()
    {
        assert(replInfo.get());
        return replInfo.get();
    }

  private:
    int serverFd;
    int masterFd;
    int port;
    std::vector<std::thread> workerThreads;
    std::unique_ptr<KeyValueStore> keyValueStore;
    std::unique_ptr<CommandDispatcher> cmdDispatcher;
    std::unique_ptr<ReplicationInfo> replInfo;

    bool createServerSocket();
    bool bindServer();
    bool listenForConnections();
    void acceptConnections();
    void handleConnection(int clientFd);

    std::optional<std::string> readFromSocket(int client_fd);
};