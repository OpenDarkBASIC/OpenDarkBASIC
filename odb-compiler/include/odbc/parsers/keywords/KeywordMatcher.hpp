#pragma once

#include "odbc/config.hpp"
#include <string>
#include <vector>

namespace odbc {

class KeywordDB;

class KeywordMatcher
{
public:
    void updateFromDB(const KeywordDB* db);
    bool findLongestKeywordMatching(const char* str, int* matchedLen) const;

private:
    std::vector<std::string> keywords_;
};

}
