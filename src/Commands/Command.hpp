#pragma once

#include "../ResponseBuilder.hpp"

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <vector>

class RedisServer;

class Command
{
  public:
    virtual void execute(int clientFd,
                         const std::vector<std::string>& command,
                         RedisServer* serverInstance = nullptr) = 0;

    void onInvalidArgs(int client_fd, const std::vector<std::string>& command)
    {
        auto response = ResponseBuilder::error(
            "ERR wrong number of arguments for '" + command[0] + "' command");
        if (send(client_fd, response.c_str(), response.length(), 0) < 0)
        {
            std::cerr << "Could not send ERROR response to client\n";
        }
    }
};