#pragma once

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
        return masterReplOffset;
    }

    inline void setMasterReplOffset(std::size_t newMasterReplOffset)
    {
        masterReplOffset = newMasterReplOffset;
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
    std::size_t masterReplOffset {0};
    std::vector<int> replicaFdVector;

    std::string generateMasterID();
};