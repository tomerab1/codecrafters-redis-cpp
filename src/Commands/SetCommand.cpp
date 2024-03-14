#include "SetCommand.hpp"

#include "pch.hpp"

#include <iostream>

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
        assert(serverInstance != nullptr);
        if (serverInstance->getReplInfo()->getRole() == "master")
        {
            serverInstance->addCommandToBuffer(command);
        }

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

        assert(serverInstance != nullptr);
        if (serverInstance->getReplInfo()->getRole() == "slave" &&
            serverInstance->getReplInfo()->getMasterFd() == clientFd &&
            serverInstance->getReplInfo()->getFinishedHandshake())
        {
            return;
        }
        onSend(clientFd, response, "Could not send SET response to client");
    }
}