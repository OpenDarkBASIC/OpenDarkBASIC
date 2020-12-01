#pragma once

#include <memory>
#include "odb-sdk/runtime/config.hpp"

namespace odb {

class KeywordDB;

class Plugin
{
public:
    Plugin() = delete;
    Plugin(const Plugin& other) = delete;
    Plugin(Plugin&& other) = default;
    ~Plugin();

    static std::unique_ptr<Plugin> load(const char* filename);

    bool loadKeywords(KeywordDB* db) const;

private:
    Plugin(void* handle) : handle_(handle) {}
    void* handle_;
};

}
