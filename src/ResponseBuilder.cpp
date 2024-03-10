#include "ResponseBuilder.hpp"

#include <sstream>

std::string ResponseBuilder::ok()
{
    return "+ok\r\n";
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

std::string ResponseBuilder::array(const std::vector<std::string>& bulkStrings)
{
    std::stringstream ss;

    ss << "*" << std::to_string(bulkStrings.size()) << "\r\n";

    for (auto& s : bulkStrings)
    {
        ss << bulkString(s);
    }

    return ss.str();
}