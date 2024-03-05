#include <chrono>
#include <iostream>
#include <string.h>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

using namespace std::chrono_literals;

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
    static bool handleRESPArray(std::string& command)
    {
        command = command.substr(1);
        handleRESPCRLF(command);
        return true;
    }

    static void handleRESPCRLF(std::string& command)
    {
        auto crlfIndex = command.find("\r\n");
        command = command.substr(crlfIndex + 2);
    }

    static std::string handleRESPBulkString(std::string& command)
    {
        std::string str;
        handleRESPCRLF(command);

        auto it = command.begin();
        std::size_t toAdvance = 0;
        while (*it != '\r' && *it != '\n' && it != command.end())
        {
            str += *it;
            it++;
            toAdvance++;
        }

        command = command.substr(toAdvance);
        return str;
    }

    static std::vector<std::string> parseCommand(const std::string& command)
    {
        std::string commandCpy = command;
        auto type = mapByteToType[commandCpy.substr(0, 1)];
        std::vector<std::string> res;
        bool isUnknownByte = false;

        while (!commandCpy.empty() && !isUnknownByte)
        {
            switch (type)
            {
                case RESPDataType::ARRAY:
                    handleRESPArray(commandCpy);
                    break;
                case RESPDataType::BULK_STRING:
                    res.emplace_back(handleRESPBulkString(commandCpy));
                    break;
                case RESPDataType::CRLF:
                    handleRESPCRLF(commandCpy);
                    break;
                default:
                    std::cout << "Unexpected byte " << commandCpy << '\n';
                    isUnknownByte = true;
                    break;
            }
            auto nextByte = commandCpy.substr(0, 1);
            auto nextTwoBytes = commandCpy.substr(0, 2);

            if (mapByteToType.find(nextByte) != mapByteToType.end())
            {
                type = mapByteToType.find(nextByte)->second;
            }
            else if (mapByteToType.find(nextTwoBytes) != mapByteToType.end())
            {
                type = mapByteToType.find(nextTwoBytes)->second;
            }
        }

        return res;
    }
};
