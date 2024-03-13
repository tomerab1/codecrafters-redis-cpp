#include "InvalidCommand.hpp"

#include "pch.hpp"

#include <iostream>

void InvalidCommand::execute(int clientFd,
                             const std::vector<std::string>& command,
                             RedisServer* serverInstance)
{
    auto response = "";
    // ResponseBuilder::error("ERR '" + command[0] + "' is not valid command");
    onSend(clientFd, response, "Could not send ERROR response to client");
}