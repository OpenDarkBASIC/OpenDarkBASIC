#pragma once

#include "odbc/config.hpp"
#include <string>
#include <vector>

namespace odbc {

class KeywordDB;

class KeywordMatcher
{
public:
    void loadFromDB(const KeywordDB* db);
    int findLongestKeywordMatching(const char* str) const;

private:
    std::vector<std::string> keywords_;
};

}
