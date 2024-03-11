#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Command;
class EchoCommand;
class GetCommand;
class InfoCommand;
class InvalidCommand;
class PSyncCommand;
class PingCommand;
class ReplConfCommand;
class SetCommand;
class RedisServer;

class CommandDispatcher
{
  public:
    CommandDispatcher();
    void dispatch(int clientFd,
                  const std::vector<std::string>& commandVec,
                  RedisServer* serverInstance);

  private:
    std::unordered_map<std::string, std::shared_ptr<Command> > strToCommandMap;
};