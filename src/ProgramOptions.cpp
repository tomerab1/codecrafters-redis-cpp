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
                    progOptionMap[argv[i]].value = argv[i + 1];
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