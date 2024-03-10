#include "EchoCommand.hpp"

#include "../KeyValueStore.hpp"
#include "../RedisServer.hpp"

void EchoCommand::execute(int clientFd,
                          const std::vector<std::string>& command,
                          RedisServer* serverInstance)
{
    if (command.size() < 2)
    {
        onInvalidArgs(clientFd, command);
    }
    std::string response = ResponseBuilder::bulkString(command[1]);
    if (send(clientFd, response.data(), response.length(), 0) < 0)
    {
        std::cerr << "Could not send ECHO response to client\n";
    }
}