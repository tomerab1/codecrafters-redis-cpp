#include "EchoCommand.hpp"

#include "pch.hpp"

void EchoCommand::execute(int clientFd,
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

        std::string response = ResponseBuilder::bulkString(command[1]);
        onSend(clientFd, response, "Could not send ECHO response to client");
    }
}
