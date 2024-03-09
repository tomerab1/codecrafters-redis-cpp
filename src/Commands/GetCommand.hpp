#pragma once

#include "Command.hpp"

class GetCommand : public Command
{
  public:
    void execute(int clientFd,
                 const std::vector<std::string>& command,
                 RedisServer* serverInstance) override;
};