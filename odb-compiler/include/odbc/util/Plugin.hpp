#pragma once

#include "odbc/config.hpp"

namespace odbc {

class KeywordDB;

class Plugin
{
public:
    Plugin() = delete;
    Plugin(const Plugin& other) = delete;
    Plugin(Plugin&& other) = default;
    ~Plugin();

    static Plugin* load(const char* filename);

    bool loadKeywords(KeywordDB* db) const;

private:
    Plugin(void* handle) : handle_(handle) {}
    void* handle_;
};

}
