#pragma once

#include <atomic>
#include <iostream>
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

    inline void addToMasterReplOffset(std::size_t inc)
    {
        masterReplOffset += inc;
    }

    inline std::size_t getMasterReplOffset()
    {
        return masterReplOffset;
    }

    inline void setFinishedHandshake(bool isDone)
    {
        // finishedHandshake.store(isDone);
        finishedHandshake = isDone;
    }

    inline bool getFinishedHandshake()
    {
        // return finishedHandshake.load();
        return finishedHandshake;
    }

    inline void setMasterFd(int fd)
    {
        masterFd = fd;
    }

    inline int getMasterFd()
    {
        return masterFd;
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
    bool finishedHandshake {false};
    std::size_t masterReplOffset {0};
    std::vector<int> replicaFdVector;

    std::string generateMasterID();
};