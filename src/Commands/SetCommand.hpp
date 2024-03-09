#pragma once

#include "Command.hpp"

class SetCommand : public Command
{
  public:
    void execute(int clientFd,
                 const std::vector<std::string>& command,
                 RedisServer* serverInstance) override;
};