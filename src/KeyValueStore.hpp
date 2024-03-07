#pragma once
#include "ResponseBuilder.hpp"

#include <mutex>
#include <string>
#include <unordered_map>

class KeyValueStore
{
  public:
    std::string set(const std::string& key, const std::string& value)
    {
        std::unique_lock<std::mutex> lk(mtx);
        keyValueStore[key] = value;
        return ResponseBuilder::ok();
    }

    std::string get(const std::string& key)
    {
        auto value = keyValueStore.find(key);
        if (value == keyValueStore.end())
        {
            return ResponseBuilder::nil();
        }

        return ResponseBuilder::bulkString(value->second);
    }

  private:
    std::mutex mtx;
    std::unordered_map<std::string, std::string> keyValueStore;
};