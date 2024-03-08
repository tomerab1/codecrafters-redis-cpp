#pragma once

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

class Parser
{
  private:
    enum class RESPDataType
    {
        SIMPLE_STRING,
        SIMPLE_ERROR,
        INTEGERS,
        BULK_STRING,
        ARRAY,
        NULLS,
        BOOLEANS,
        DOUBLES,
        BIG_NUM,
        BULK_ERROR,
        VERBATIM_STRINGS,
        MAPS,
        SETS,
        PUSHES,
        CRLF,
    };

    inline static std::unordered_map<std::string, RESPDataType> mapByteToType =
        {{"+", RESPDataType::SIMPLE_STRING},
         {"-", RESPDataType::SIMPLE_ERROR},
         {":", RESPDataType::INTEGERS},
         {"$", RESPDataType::BULK_STRING},
         {"*", RESPDataType::ARRAY},
         {"_", RESPDataType::NULLS},
         {"#", RESPDataType::BOOLEANS},
         {",", RESPDataType::DOUBLES},
         {"(", RESPDataType::BIG_NUM},
         {"!", RESPDataType::BULK_ERROR},
         {"=", RESPDataType::VERBATIM_STRINGS},
         {"%", RESPDataType::MAPS},
         {"~", RESPDataType::SETS},
         {">", RESPDataType::PUSHES},
         {"\r\n", RESPDataType::CRLF}};

  public:
    static bool handleRESPArray(std::string& command);
    static void handleRESPCRLF(std::string& command);
    static std::string handleRESPBulkString(std::string& command);
    static std::vector<std::string> parseCommand(const std::string& command);
};
