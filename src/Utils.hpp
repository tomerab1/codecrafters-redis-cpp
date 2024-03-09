#pragma once

#include <sstream>
#include <string>
#include <vector>

class Utils
{
  public:
    static std::vector<std::string> split(const std::string& str, char sep)
    {
        std::string temp;
        std::stringstream ss {str};
        std::vector<std::string> res;

        while (std::getline(ss, temp, sep))
        {
            res.emplace_back(std::move(temp));
        }

        return res;
    }
};