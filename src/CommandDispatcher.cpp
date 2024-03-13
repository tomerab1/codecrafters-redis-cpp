#include "CommandDispatcher.hpp"

#include "Commands/EchoCommand.hpp"
#include "Commands/GetCommand.hpp"
#include "Commands/InfoCommand.hpp"
#include "Commands/InvalidCommand.hpp"
#include "Commands/PSyncCommand.hpp"
#include "Commands/PingCommand.hpp"
#include "Commands/ReplConfCommand.hpp"
#include "Commands/SetCommand.hpp"
#include "RedisServer.hpp"
#include "Replication/ReplicationInfo.hpp"

CommandDispatcher::CommandDispatcher() :
    strToCommandMap {{
        {"echo", std::make_unique<EchoCommand>()},
        {"ping", std::make_unique<PingCommand>()},
        {"set", std::make_unique<SetCommand>()},
        {"get", std::make_unique<GetCommand>()},
        {"info", std::make_unique<InfoCommand>()},
        {"psync", std::make_unique<PSyncCommand>()},
        {"replconf", std::make_unique<ReplConfCommand>()},
        {"invalid", std::make_unique<InvalidCommand>()},
    }}
{}

void CommandDispatcher::dispatch(int clientFd,
                                 const std::vector<std::string>& commandVec,
                                 RedisServer* serverInstance)
{
    assert(serverInstance != nullptr);

    auto isFound = strToCommandMap.find(commandVec[0]);
    if (isFound == strToCommandMap.end())
    {
        strToCommandMap["invalid"]->execute(
            clientFd, commandVec, serverInstance);
    }
    else
    {
        isFound->second->execute(clientFd, commandVec, serverInstance);
        if (serverInstance->getReplInfo()->getRole() == "slave" &&
            serverInstance->getReplInfo()->getFinishedHandshake())
        {
            auto rawCommand = ResponseBuilder::array(commandVec);
            serverInstance->getReplInfo()->addToMasterReplOffset(
                rawCommand.length());
        }
    }
