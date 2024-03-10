#include "ReplicationInfo.hpp"

#include <random>
#include <sstream>

ReplicationInfo::ReplicationInfo(const std::string& role) : role {role}
{
    if (role == "master")
    {
        masterReplId = generateMasterID();
    }
}

std::string ReplicationInfo::generateMasterID()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);

    std::stringstream ss;
    const char hexDigits[] = "0123456789abcdef";
    for (int i = 0; i < 40; ++i)
    {
        ss << hexDigits[dis(gen)];
    }

    return ss.str();
}

std::string ReplicationInfo::toString()
{
    std::stringstream ss;

    ss << "role:" << role << "\r\n";
    ss << "master_replid:" << masterReplId << "\r\n";
    ss << "master_repl_offset:" << masterReplOffset << "\r\n";

    return ss.str();
}