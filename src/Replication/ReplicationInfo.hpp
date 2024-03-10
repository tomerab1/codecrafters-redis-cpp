#pragma once

#include <string>

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

    std::string toString();

  private:
    std::string role;
    std::string masterReplId;
    std::size_t masterReplOffset {0};

    std::string generateMasterID();
};