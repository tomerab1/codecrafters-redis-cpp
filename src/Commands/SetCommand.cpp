#include "SetCommand.hpp"

#include "../KeyValueStore.hpp"
#include "../RedisServer.hpp"

#include <algorithm>

void SetCommand::execute(int clientFd,
                         const std::vector<std::string>& command,
                         RedisServer* serverInstance)
{
    if (command.size() < 3)
    {
        onInvalidArgs(clientFd, command);
    }
    else
    {
        std::string response = "";
        if (std::find(command.begin(), command.end(), "px") == command.end())
        {
            response =
                serverInstance->getKVStore()->set(command[1], command[2]);
        }
        else
        {
            if (command.size() < 4)
            {
                onInvalidArgs(clientFd, command);
            }
            else
            {
                response = serverInstance->getKVStore()->set(
                    command[1], command[2], command[4]);
            }
        }
        if (send(clientFd, response.data(), response.length(), 0) < 0)
        {
            std::cerr << "Could not send SET response to client\n";
        }
    }
}