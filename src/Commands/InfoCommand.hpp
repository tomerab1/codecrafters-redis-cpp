#pragma once

#include "Command.hpp"

class InfoCommand : public Command
{
  public:
    void execute(int clientFd,
                 const std::vector<std::string>& command,
                 RedisServer* kvStore) override;
};