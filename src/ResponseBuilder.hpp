#pragma once
#include <format>
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
        return std::format("-{}\r\n", err);
    }

    static std::string bulkString(const std::string& str)
    {
        return std::format("${}\r\n{}\r\n", std::to_string(str.length()), str);
    }
};