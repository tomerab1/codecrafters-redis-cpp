#include "ProgramOptions.hpp"
#include "RedisServer.hpp"

#include <any>
#include <string>

std::any strToInt(std::string_view s)
{
    return std::stoi(std::string(s));
}

int main(int argc, char** argv)
{
    ProgramOptions po({ProgramOptions::Option {
        .shortName = "-p",
        .longName = "--port",
        .transformFn = strToInt,
        .defaultValue = 6379,
    }});

    try
    {
        po.parse(argc, argv);

        int port;
        if (po.hasOption("--port"))
        {
            port = po.get<int>("--port").value();
        }

        RedisServer server(port);
        server.start();
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << "\n";
    }

    return 0;
}
