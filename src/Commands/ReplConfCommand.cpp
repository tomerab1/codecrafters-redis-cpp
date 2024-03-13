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
        assert(serverInstance != nullptr);

        if (command[1] == "getack")
        {
            auto response = ResponseBuilder::array(
                {"replconf",
                 "ack",
                 std::to_string(
                     serverInstance->getReplInfo()->getMasterReplOffset())});
            onSend(clientFd, response, "Could not send REPLCONF ACK to master");
        }
        else
        {
            if (serverInstance->getReplInfo()->getRole() == "slave" &&
                serverInstance->getReplInfo()->getMasterFd() == clientFd &&
                serverInstance->getReplInfo()->getFinishedHandshake())
            {
                return;
            }

            auto response = ResponseBuilder::ok();
            onSend(clientFd,
                   response,
                   "Could not send REPLCONF response to client");
        }
    }
}