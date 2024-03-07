#pragma once
#include <string>

class ResponseBuilder
{
  public:
    static std::string ok()
    {
        return "+OK\r\n";
    }

    static std::string nil()
    {
        return "$-1\r\n";
    }

    static std::string error(const std::string& err)
    {
        return "-" + err + "\r\n";
    }

    static std::string bulkString(const std::string& str)
    {
        return "$" + std::to_string(str.length()) + "\r\n" + str + "\r\n";
    }
};