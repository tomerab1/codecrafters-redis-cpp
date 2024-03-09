#include "ProgramOptions.hpp"

ProgramOptions::ProgramOptions(std::initializer_list<Option> options)
{
    for (auto opt : options)
    {
        if (opt.shortName != "")
        {
            progOptionMap[opt.shortName] = opt;
        }
        if (opt.longName != "")
        {
            progOptionMap[opt.longName] = opt;
        }
        if (opt.shortName == "" && opt.longName == "")
        {
            throw std::logic_error(
                "Error: program option must have short/long name");
        }
    }
}

bool ProgramOptions::hasOption(std::string_view optionName)
{
    return progOptionMap.find(optionName) != progOptionMap.end();
}

void ProgramOptions::parse(int argc, char** argv)
{
    for (int i = 0; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            std::string_view argvAsSw(argv[i]);
            auto isKey = progOptionMap.find(argvAsSw);
            if (isKey == progOptionMap.end())
            {
                unidentifiedArgs.emplace_back(argvAsSw);
            }
            else
            {
                if (i + 1 < argc)
                {
                    isKey->second.value = "";
                    for (int j = 1; j <= progOptionMap[argv[i]].numOfParams;
                         j++)
                    {
                        if (i + j > argc)
                        {
                            throw std::logic_error("To many arguments for \"" +
                                                   std::string(argv[i]) + "\"");
                        }

                        progOptionMap[argv[i]].value.value() +=
                            std::string(argv[i + j]) + ",";
                    }
                }
                else
                {
                    throw std::logic_error("Error: value expected after \"" +
                                           std::string(argv[i]) + "\"");
                }
            }
        }
    }
}