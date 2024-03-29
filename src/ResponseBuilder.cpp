#include "ResponseBuilder.hpp"

#include <sstream>

std::string ResponseBuilder::ok()
{
    return "+OK\r\n";
}

std::string ResponseBuilder::nil()
{
    return "$-1\r\n";
}

std::string ResponseBuilder::fullresync(const std::string str)
{
    return "+FULLRESYNC " + str + "\r\n";
}

std::string ResponseBuilder::respInt(int val)
{
    return ":" + std::to_string(val) + "\r\n";
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

std::string ResponseBuilder::emptyRDB()
{
    unsigned char emptyRdbFile[] = {
        0x52, 0x45, 0x44, 0x49, 0x53, 0x30, 0x30, 0x31, 0x31,
        0xfa, 0x09, 0x72, 0x65, 0x64, 0x69, 0x73, 0x2d,

        0x76, 0x65, 0x72, 0x05, 0x37, 0x2e, 0x32, 0x2e, 0x30,
        0xfa, 0x0a, 0x72, 0x65, 0x64, 0x69, 0x73, 0x2d,

        0x62, 0x69, 0x74, 0x73, 0xc0, 0x40, 0xfa, 0x05, 0x63,
        0x74, 0x69, 0x6d, 0x65, 0xc2, 0x6d, 0x08, 0xbc,

        0x65, 0xfa, 0x08, 0x75, 0x73, 0x65, 0x64, 0x2d, 0x6d,
        0x65, 0x6d, 0xc2, 0xb0, 0xc4, 0x10, 0x00, 0xfa,

        0x08, 0x61, 0x6f, 0x66, 0x2d, 0x62, 0x61, 0x73, 0x65,
        0xc0, 0x00, 0xff, 0xf0, 0x6e, 0x3b, 0xfe, 0xc0,

        0xff, 0x5a, 0xa2};

    std::string asStr = "";
    for (auto ch : emptyRdbFile)
    {
        asStr += ch;
    }

    auto bulkStr = bulkString(asStr);
    return bulkStr.substr(0, bulkStr.length() - 2);
}