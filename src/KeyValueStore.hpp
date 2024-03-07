#pragma once
#include "ResponseBuilder.hpp"

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
    std::string set(const std::string& key, const std::string& value)
    {
        std::unique_lock<std::mutex> lk(mtx);
        keyValueStore[key] = {.value = ValueEntry::ValueWithoutExpiry(value)};
        return ResponseBuilder::ok();
    }

    std::string set(const std::string& key,
                    const std::string& value,
                    const std::string& expiryMs)
    {
        std::unique_lock<std::mutex> lk(mtx);
        auto expiryDuration = std::chrono::milliseconds(std::stol(expiryMs));
        auto expiryTimePoint =
            std::chrono::system_clock::now() + expiryDuration;

        keyValueStore[key] = {
            .value = ValueEntry::ValueWithExpiry(value, expiryTimePoint)};
        return ResponseBuilder::ok();
    }

    std::string get(const std::string& key)
    {
        auto value = keyValueStore.find(key);
        if (value == keyValueStore.end())
        {
            return ResponseBuilder::nil();
        }

        std::string strValue = "";
        if (std::holds_alternative<ValueEntry::ValueWithExpiry>(
                value->second.value))
        {
            auto vwe =
                std::get<ValueEntry::ValueWithExpiry>(value->second.value);

            if (vwe.tp - std::chrono::system_clock::now() <=
                std::chrono::milliseconds::zero())
            {
                keyValueStore.erase(key);
                return ResponseBuilder::nil();
            }
            else
            {
                strValue = vwe.value;
            }
        }
        else
        {
            strValue =
                std::get<ValueEntry::ValueWithoutExpiry>(value->second.value)
                    .value;
        }

        return ResponseBuilder::bulkString(strValue);
    }

  private:
    std::mutex mtx;
    std::unordered_map<std::string, ValueEntry> keyValueStore;
};