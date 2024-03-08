#pragma once

#include <any>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <vector>

class ProgramOptions
{
  public:
    struct Option
    {
        std::string_view shortName {""};
        std::string_view longName {""};
        std::string_view value {""};
        std::optional<std::function<std::any(std::string_view)> > transformFn {
            std::nullopt};
        std::optional<std::any> defaultValue {std::nullopt};
    };

  private:
    std::unordered_map<std::string_view, Option> progOptionMap;
    std::vector<std::string_view> unidentifiedArgs;

  public:
    ProgramOptions(std::initializer_list<Option> options);

    void parse(int argc, char** argv);

    bool hasOption(std::string_view optionName);

    template<typename T>
    std::optional<T> get(std::string_view optionName)
    {
        auto value = progOptionMap.find(optionName);
        if (value == progOptionMap.end())
        {
            throw std::logic_error("Error: program option \"" +
                                   std::string(optionName) + "\" " +
                                   "does not exist");
        }

        if (value->second.transformFn)
        {
            try
            {
                return std::any_cast<T>(
                    value->second.transformFn.value()(value->second.value));
            }
            catch (const std::bad_any_cast& e)
            {
                throw e;
            }
        }
        if (value->second.defaultValue)
        {
            try
            {
                return std::any_cast<T>(value->second.defaultValue.value());
            }
            catch (const std::bad_any_cast& e)
            {
                throw e;
            }
        }

        return std::nullopt;
    }
};