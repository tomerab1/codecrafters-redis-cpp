#include "PSyncCommand.hpp"

#include "../KeyValueStore.hpp"
#include "../RedisServer.hpp"
#include "../Replication/ReplicationInfo.hpp"

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
        std::cerr << "Could not send PONG to client\n";
    }
}