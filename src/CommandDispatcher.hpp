#pragma once

#include "Commands/EchoCommand.hpp"
#include "Commands/GetCommand.hpp"
#include "Commands/InfoCommand.hpp"
#include "Commands/InvalidCommand.hpp"
#include "Commands/PSyncCommand.hpp"
#include "Commands/PingCommand.hpp"
#include "Commands/ReplConfCommand.hpp"
#include "Commands/SetCommand.hpp"
#include "RedisServer.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class CommandDispatcher
{
  public:
    inline void dispatch(int clientFd,
                         const std::vector<std::string>& commandVec,
                         RedisServer* serverInstance)
    {
        auto isFound = strToCommandMap.find(commandVec[0]);
        if (isFound == strToCommandMap.end())
        {
            strToCommandMap["invalid"]->execute(
                clientFd, commandVec, serverInstance);
        }
        else
        {
            isFound->second->execute(clientFd, commandVec, serverInstance);
        }
    }

  private:
    std::unordered_map<std::string, std::shared_ptr<Command> > strToCommandMap {
        {
            {"echo", std::make_unique<EchoCommand>()},
            {"ping", std::make_unique<PingCommand>()},
            {"set", std::make_unique<SetCommand>()},
            {"get", std::make_unique<GetCommand>()},
            {"info", std::make_unique<InfoCommand>()},
            {"psync", std::make_unique<PSyncCommand>()},
            {"replconf", std::make_unique<ReplConfCommand>()},
            {"invalid", std::make_unique<InvalidCommand>()},
        }};
};