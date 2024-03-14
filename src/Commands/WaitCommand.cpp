#include "WaitCommand.hpp"

#include "pch.hpp"

void WaitCommand::execute(int clientFd,
                          const std::vector<std::string>& command,
                          RedisServer* serverInstance)
{
    if (command.size() < 3)
    {
        onInvalidArgs(clientFd, command);
    }
    else
    {
        auto response = ResponseBuilder::respInt(0);
        onSend(clientFd, response, "Could not send WAIT response to client");
    }
}
