#include "ReplConfCommand.hpp"

#include "pch.hpp"

#include <iostream>

void ReplConfCommand::execute(int clientFd,
                              const std::vector<std::string>& command,
                              RedisServer* serverInstance)
{
    if (command.size() < 3)
    {
        onInvalidArgs(clientFd, command);
    }
    else
    {
        if (command[1] == "getack")
        {
            auto response = ResponseBuilder::array({"replconf", "ack", "0"});
            onSend(clientFd, response, "Could not send REPLCONF ACK to master");
        }
        else
        {
            auto response = ResponseBuilder::ok();

            onSend(clientFd,
                   response,
                   "Could not send REPLCONF response to client");
        }
    }
}