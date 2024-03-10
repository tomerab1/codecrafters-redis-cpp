#pragma once

#include "Commands/EchoCommand.hpp"
#include "Commands/GetCommand.hpp"
#include "Commands/InfoCommand.hpp"
#include "Commands/InvalidCommand.hpp"
#include "Commands/PSyncCommand.hpp"
#include "Commands/PingCommand.hpp"
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
            {"echo", std::make_shared<EchoCommand>()},
            {"ping", std::make_shared<PingCommand>()},
            {"set", std::make_shared<SetCommand>()},
            {"get", std::make_shared<GetCommand>()},
            {"info", std::make_shared<InfoCommand>()},
            {"psync", std::make_shared<PSyncCommand>()},
            {"invalid", std::make_shared<InvalidCommand>()},
        }};
};