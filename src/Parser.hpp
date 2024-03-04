#include <iostream>
#include <string.h>
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

    static constexpr std::size_t LEN_CRLF_BYTES = 2;

  public:
    static std::vector<std::string> parseCommand(const std::string& command)
    {
        const char* rawStr = command.c_str();
        auto type = mapByteToType[std::string(1, *rawStr)];
        std::vector<std::string> res;
        int numOfArguments = 0;
        bool isUnknownByte = false;

        while (*rawStr && !isUnknownByte)
        {
            switch (type)
            {
                case RESPDataType::BULK_STRING: {
                    rawStr++;
                    std::size_t afterInt = 0;
                    numOfArguments = std::stoi(rawStr, &afterInt);
                    rawStr += afterInt + LEN_CRLF_BYTES;
                    std::string bulkString;

                    while (*rawStr && *(rawStr + 1) && *rawStr != '\r' &&
                           *(rawStr + 1) != '\n')
                    {
                        bulkString += *rawStr;
                        rawStr++;
                    }

                    res.emplace_back(std::move(bulkString));
                }
                break;
                case RESPDataType::ARRAY: {
                    rawStr++;
                    std::size_t afterInt = 0;
                    numOfArguments = std::stoi(rawStr, &afterInt);
                    rawStr += afterInt + LEN_CRLF_BYTES;
                }
                break;
                case RESPDataType::CRLF:
                    rawStr += LEN_CRLF_BYTES;
                    break;
                default:
                    std::cout << "Unexpected byte " << *(rawStr + 1) << '\n';
                    isUnknownByte = true;
                    break;
            }
            if (mapByteToType.find(std::string(1, *rawStr)) !=
                mapByteToType.end())
            {
                type = mapByteToType.find(std::string(1, *rawStr))->second;
            }
            else if (mapByteToType.find(std::string(1, *rawStr) +
                                        std::string(1, *(rawStr + 1))) !=
                     mapByteToType.end())
            {
                type = mapByteToType
                           .find(std::string(1, *rawStr) +
                                 std::string(1, *(rawStr + 1)))
                           ->second;
            }
            else
            {
                isUnknownByte = true;
            }
        }

        return res;
    }
};
