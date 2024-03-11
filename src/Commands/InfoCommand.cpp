#include "InfoCommand.hpp"

#include "../KeyValueStore.hpp"
#include "../RedisServer.hpp"
#include "../Replication/ReplicationInfo.hpp"

#include <algorithm>

void InfoCommand::execute(int clientFd,
                          const std::vector<std::string>& command,
                          RedisServer* serverInstance)
{
    if (command.size() < 1)
    {
        onInvalidArgs(clientFd, command);
    }
    else
    {
        // serverInstance->getReplInfo()->toString();
        auto toEncode = "";
        auto response = ResponseBuilder::bulkString(toEncode);

        if (send(clientFd, response.data(), response.length(), 0) < 0)
        {
            std::cerr << "Could not send INFO response to client\n";
        }
    }
}