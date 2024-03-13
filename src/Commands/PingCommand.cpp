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
        onSend(clientFd, PONG_STR, "Could not send PONG to client");
    }
}