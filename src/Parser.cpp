#include "Parser.hpp"

bool Parser::handleRESPArray(std::string& command)
{
    handleRESPCRLF(command);
    return true;
}

void Parser::handleRESPCRLF(std::string& command)
{
    auto crlfIndex = command.find("\r\n");
    if (crlfIndex == std::string::npos)
    {
        throw std::invalid_argument("CRLF not found");
    }
    command = command.substr(crlfIndex + 2);
}

std::string Parser::handleRESPBulkString(std::string& command)
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

std::vector<std::string> Parser::parseCommand(const std::string& command)
{
    std::string commandCpy = command;
    auto type = mapByteToType[commandCpy.substr(0, 1)];
    std::vector<std::string> res;
    bool isUnknownByte = false;

    std::transform(
        commandCpy.begin(), commandCpy.end(), commandCpy.begin(), ::tolower);

    try
    {
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
                    std::cerr << "Unexpected byte " << commandCpy << '\n';
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
            else
            {
                isUnknownByte = true;
            }
        }

        return res;
    }
    catch (const std::invalid_argument& e)
    {
        std::cerr << "Error parsing command: " << e.what() << '\n';
        return {};
    }
}