#include "PSyncCommand.hpp"

#include "pch.hpp"

void PSyncCommand::execute(int clientFd,
                           const std::vector<std::string>& command,
                           RedisServer* serverInstance)
{
    auto masterId = serverInstance->getReplInfo()->getMasterReplId();
    auto masterOffset =
        std::to_string(serverInstance->getReplInfo()->getMasterReplOffset());

    auto response = ResponseBuilder::fullresync(masterId + " " + masterOffset);

    if (send(clientFd, response.c_str(), response.length(), 0) < 0)
    {
        std::cerr << "Could not send PSYNC to client\n";
    }

    auto emptyRDB = ResponseBuilder::emptyRDB();
    if (send(clientFd, emptyRDB.c_str(), emptyRDB.length(), 0) < 0)
    {
        std::cerr << "Could not send EMPTY_RDB to client\n";
    }
}