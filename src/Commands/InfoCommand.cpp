#include "InfoCommand.hpp"

#include "pch.hpp"

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
        auto toEncode = serverInstance->getReplInfo()->toString();
        auto response = ResponseBuilder::bulkString(toEncode);
        onSend(clientFd, response, "Could not send INFO response to client");
    }
}