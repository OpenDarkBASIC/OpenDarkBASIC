#pragma once

#include "odbc/config.hpp"
#include "odbc/parsers/keywords/Keyword.hpp"
#include <unordered_map>

namespace odbc {

class KeywordDB
{
public:
    bool loadFromDirectory(const std::string& dir);
    bool appendFromFile(const std::string& fileName);
    bool exists(const std::string& keyword);

    bool addKeyword(Keyword keyword);
    const Keyword* lookup(const std::string& keyword) const;
    void printAll();

private:
    std::unordered_map<std::string, Keyword> map_;
};

}
