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
        assert(serverInstance != nullptr);
        if (serverInstance->getReplInfo()->getRole() == "slave" &&
            serverInstance->getReplInfo()->getMasterFd() == clientFd &&
            serverInstance->getReplInfo()->getFinishedHandshake())
        {
            return;
        }

        auto toEncode = serverInstance->getReplInfo()->toString();
        auto response = ResponseBuilder::bulkString(toEncode);
        onSend(clientFd, response, "Could not send INFO response to client");
    }
}