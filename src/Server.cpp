#include "RedisServer.hpp"

int main(int argc, char** argv)
{
    RedisServer server(6379);
    server.start();

    return 0;
}
