#pragma once

#include "odbc/config.hpp"
#include <string>
#include <vector>

namespace odbc {

class KeywordDB;

class ODBC_PUBLIC_API KeywordMatcher
{
public:
    void updateFromDB(const KeywordDB* db);
    bool findLongestKeywordMatching(const char* str, int* matchedLen) const;
    int longestKeywordLength() const;

private:
    std::vector<std::string> keywords_;
    int longestKeywordLength_ = 0;
};

}
