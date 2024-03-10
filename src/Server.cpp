#include "ProgramOptions.hpp"
#include "RedisServer.hpp"
#include "Utils.hpp"

#include <any>
#include <string>

struct ReplicaOfParams
{
    std::string hostName;
    int port;
};

std::any strToInt(std::string s)
{
    std::size_t idx;
    return std::stoi(std::string(s), &idx);
}

std::any replicaOfToVec(std::string s)
{
    auto splitted = Utils::split(s, ',');
    if (splitted.size() < 2)
    {
        throw std::logic_error(
            "replicaOf command should be followed by 2 arguments");
    }

    return ReplicaOfParams {.hostName = splitted[0],
                            .port = std::stoi(splitted[1])};
}

int main(int argc, char** argv)
{
    ProgramOptions po({ProgramOptions::Option {
                           .shortName = "-p",
                           .longName = "--port",
                           .transformFn = strToInt,
                           .defaultValue = 6379,
                       },
                       ProgramOptions::Option {
                           .longName = "--replicaof",
                           .numOfParams = 2,
                           .transformFn = replicaOfToVec,
                       }});

    try
    {
        po.parse(argc, argv);

        int port;
        bool isMaster {true};
        ReplicaOfParams replicaof;

        if (po.hasOption("--port"))
        {
            auto value = po.get<int>("--port");
            if (value.has_value())
            {
                port = value.value();
            }
        }
        if (po.hasOption("--replicaof"))
        {
            auto value = po.get<ReplicaOfParams>("--replicaof");
            if (value.has_value())
            {
                isMaster = false;
            }
        }

        std::cout << "Listening on port " << port << "...\n";

        RedisServer server(port, isMaster);
        server.start();
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << "\n";
    }

    return 0;
}
