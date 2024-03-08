#pragma once

class ResponseBuilder;

#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <variant>

class KeyValueStore
{
  private:
    struct ValueEntry
    {
        struct ValueWithExpiry
        {
            std::string value;
            std::chrono::time_point<std::chrono::system_clock> tp;
        };
        struct ValueWithoutExpiry
        {
            std::string value;
        };

        std::variant<ValueWithExpiry, ValueWithoutExpiry> value;
    };

  public:
    std::string set(const std::string& key, const std::string& value);
    std::string set(const std::string& key,
                    const std::string& value,
                    const std::string& expiryMs);
    std::string get(const std::string& key);

  private:
    std::mutex mtx;
    std::unordered_map<std::string, ValueEntry> keyValueStore;
};