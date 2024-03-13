#include "PSyncCommand.hpp"

#include "pch.hpp"

#include <chrono>
#include <iostream>
#include <thread>

using namespace std::chrono_literals;

void PSyncCommand::execute(int clientFd,
                           const std::vector<std::string>& command,
                           RedisServer* serverInstance)
{
    if (command.size() < 3)
    {
        onInvalidArgs(clientFd, command);
    }
    else
    {
        assert(serverInstance != nullptr);
        assert(serverInstance->getReplInfo() != nullptr);
        assert(serverInstance->getReplInfo()->getRole() == "master");

        auto masterId = serverInstance->getReplInfo()->getMasterReplId();
        auto masterOffset = std::to_string(
            serverInstance->getReplInfo()->getMasterReplOffset());

        auto response =
            ResponseBuilder::fullresync(masterId + " " + masterOffset);

        onSend(clientFd, response, "Could not send PSYNC to client");

        auto emptyRDB = ResponseBuilder::emptyRDB();
        serverInstance->getReplInfo()->addToReplicaVector(clientFd);
        onSend(clientFd, emptyRDB, "Could not send EMPTY_RDB to client");
    }
}