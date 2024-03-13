#pragma once

#include <atomic>
#include <stdexcept>
#include <string>
#include <vector>

class ReplicationInfo
{
  public:
    explicit ReplicationInfo(const std::string& role);

    inline const std::string getRole()
    {
        return role;
    }

    inline void setRoel(const std::string& newRole)
    {
        role = newRole;
    }

    inline const std::string getMasterReplId()
    {
        return masterReplId;
    }

    inline std::size_t getMasterReplOffset()
    {
        return masterPrevReplOffset.load();
    }

    inline void addToMasterReplOffset(std::size_t inc)
    {
        masterPrevReplOffset.store(masterReplOffset.load());
        masterReplOffset += inc;
    }

    inline void setFinishedHandshake(bool isDone)
    {
        finishedHandshake.store(isDone);
    }

    inline void setMasterFd(int fd)
    {
        masterFd = fd;
    }

    inline int getMasterFd()
    {
        return masterFd;
    }

    inline bool getFinishedHandshake()
    {
        return finishedHandshake.load();
    }

    inline void addToReplicaVector(int clientFd)
    {
        if (role != "master")
        {
            throw std::logic_error(
                "Error: Attempt to add replica to vector from replica");
        }

        replicaFdVector.push_back(clientFd);
    }

    inline std::vector<int> getReplicaVector()
    {
        if (role != "master")
        {
            throw std::logic_error(
                "Error: Attempt to get replica vector from replica");
        }
        return replicaFdVector;
    }

    std::string toString();

  private:
    std::string role;
    std::string masterReplId;
    int masterFd {-1};
    std::atomic<bool> finishedHandshake {false};
    std::atomic<std::size_t> masterReplOffset {0};
    std::atomic<std::size_t> masterPrevReplOffset {0};
    std::vector<int> replicaFdVector;

    std::string generateMasterID();
};