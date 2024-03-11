#include "GetCommand.hpp"

#include "pch.hpp"

void GetCommand::execute(int clientFd,
                         const std::vector<std::string>& command,
                         RedisServer* serverInstance)
{
    if (command.size() < 2)
    {
        onInvalidArgs(clientFd, command);
    }
    else
    {
        auto response = serverInstance->getKVStore()->get(command[1]);
        if (send(clientFd, response.data(), response.length(), 0) < 0)
        {
            std::cerr << "Could not send GET response to client\n";
        }
    }
}