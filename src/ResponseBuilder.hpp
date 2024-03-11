#pragma once
#include <string>
#include <vector>

class ResponseBuilder
{
  public:
    static std::string ok();
    static std::string nil();
    static std::string fullresync(const std::string str);
    static std::string error(const std::string& err);
    static std::string bulkString(const std::string& str);
    static std::string array(const std::vector<std::string>& bulkStrings);
    static std::string emptyRDB();
};