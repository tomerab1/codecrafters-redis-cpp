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
    else
    {
        std::string response = ResponseBuilder::bulkString(command[1]);
        if (send(clientFd, response.c_str(), response.length(), 0) < 0)
        {
            std::cerr << "Could not send ECHO response to client\n";
        }
    }
}
