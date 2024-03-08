#include "ResponseBuilder.hpp"

std::string ResponseBuilder::ok()
{
    return "+OK\r\n";
}

std::string ResponseBuilder::nil()
{
    return "$-1\r\n";
}

std::string ResponseBuilder::error(const std::string& err)
{
    return "-" + err + "\r\n";
}

std::string ResponseBuilder::bulkString(const std::string& str)
{
    return "$" + std::to_string(str.length()) + "\r\n" + str + "\r\n";
}