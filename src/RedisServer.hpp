#pragma once

#include <algorithm>
#include <arpa/inet.h>
#include <array>
#include <cassert>
#include <cstring>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
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

static constexpr std::size_t COMMAND_BUFFER_SIZE = 1024;

class RedisServer
{
  public:
    RedisServer(int port, bool isMaster);
    ~RedisServer();
    void start(int masterPort = 0, const std::string& masterAddr = "");

    inline void addCommandToBuffer(const std::vector<std::string>& command)
    {
        std::unique_lock<std::mutex> lk(commandBufferMtx);
        std::cout << "adding " << command.size() << " to buffer\n";
        commandBuffer.emplace_back(command);
    }

    std::list<std::vector<std::string> > getCommandBuffer()
    {
        return commandBuffer
    }

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
    bool mShouldTerminateDistribution {false};
    std::mutex commandBufferMtx;
    std::list<std::vector<std::string> > commandBuffer;
    std::vector<std::thread> workerThreads;
    std::unique_ptr<KeyValueStore> keyValueStore;
    std::unique_ptr<CommandDispatcher> cmdDispatcher;
    std::unique_ptr<ReplicationInfo> replInfo;

    inline static RedisServer* instancePointer {nullptr};

    bool createServerSocket();
    bool bindServer();
    bool listenForConnections();
    void acceptConnections();
    void handleConnection(int clientFd);

    void connectToMaster(int masterPort, const std::string& masterAddr);
    void handshake(int masterPort, const std::string& masterAddr);
    void sendCommandToMaster(const std::string& command,
                             const std::vector<std::string>& args = {});
    void distributeCommandToReplicas(const std::string& rawCommand);
    void distributeCommandsFromBuffer(bool& shouldTerminateDistribution);
    static void handleSIGINT(int signal);
    void shutdown();

    std::optional<std::string> readFromSocket(int client_fd);
};