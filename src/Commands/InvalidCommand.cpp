#include "InvalidCommand.hpp"

#include "../RedisServer.hpp"
#include "../ResponseBuilder.hpp"

#include <iostream>

void InvalidCommand::execute(int clientFd,
                             const std::vector<std::string>& command,
                             RedisServer* serverInstance)
{
    auto response = ResponseBuilder::error("ERR \"" + command[0] +
                                           "\" is not valid command");
    if (send(clientFd, response.data(), response.length(), 0) < 0)
    {
        std::cerr << "Could not send ERROR response to client\n";
    }
}