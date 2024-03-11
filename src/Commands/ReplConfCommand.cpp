#include "ReplConfCommand.hpp"

#include "pch.hpp"

void ReplConfCommand::execute(int clientFd,
                              const std::vector<std::string>& command,
                              RedisServer* serverInstance)
{
    auto response = ResponseBuilder::ok();

    if (send(clientFd, response.data(), response.length(), 0) < 0)
    {
        std::cerr << "Could not send REPLCONF response to client\n";
    }
}