#include "InfoCommand.hpp"

#include "../KeyValueStore.hpp"
#include "../RedisServer.hpp"

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
        std::string toEncode = "role:" + serverInstance->getRole() + "\r\n";
        auto response = ResponseBuilder::bulkString(toEncode);

        if (send(clientFd, response.data(), response.length(), 0) < 0)
        {
            std::cerr << "Could not send GET response to client\n";
        }
    }
}