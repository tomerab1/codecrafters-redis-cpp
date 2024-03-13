#include "PingCommand.hpp"

#include "pch.hpp"

static constexpr std::string PONG_STR = "+PONG\r\n";

void PingCommand::execute(int clientFd,
                          const std::vector<std::string>& command,
                          RedisServer* serverInstance)
{
    if (command.size() > 1)
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

        onSend(clientFd, PONG_STR, "Could not send PONG to client");
    }
}