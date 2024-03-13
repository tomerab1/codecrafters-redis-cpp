#include "GetCommand.hpp"

#include "pch.hpp"

#include <iostream>

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
        assert(serverInstance != nullptr);
        if (serverInstance->getReplInfo()->getRole() == "slave" &&
            serverInstance->getReplInfo()->getMasterFd() == clientFd &&
            serverInstance->getReplInfo()->getFinishedHandshake())
        {
            return;
        }
        auto response = serverInstance->getKVStore()->get(command[1]);
        onSend(clientFd, response, "Could not send GET response to client");
    }
}