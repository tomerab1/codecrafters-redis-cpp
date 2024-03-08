#pragma once
#include <string>

class ResponseBuilder
{
  public:
    static std::string ok();
    static std::string nil();
    static std::string error(const std::string& err);
    static std::string bulkString(const std::string& str);
};