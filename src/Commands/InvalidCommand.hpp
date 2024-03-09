#pragma once

#include "Command.hpp"

class InvalidCommand : public Command
{
  public:
    void execute(int clientFd,
                 const std::vector<std::string>& command,
                 RedisServer* serverInstance) override;
};