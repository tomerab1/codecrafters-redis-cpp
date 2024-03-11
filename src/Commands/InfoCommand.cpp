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

        if (send(clientFd, response.data(), response.length(), 0) < 0)
        {
            std::cerr << "Could not send INFO response to client\n";
        }
    }
}