#include "PingCommand.hpp"

#include "pch.hpp"

static constexpr std::string PONG_STR = "+PONG\r\n";

void PingCommand::execute(int clientFd,
                          const std::vector<std::string>& command,
                          RedisServer* serverInstance)
{
    if (send(clientFd, PONG_STR.data(), PONG_STR.length(), 0) < 0)
    {
        std::cerr << "Could not send PONG to client\n";
    }
}