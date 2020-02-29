#pragma once

#include "odbc/config.hpp"
#include <string>
#include <string_view>
#include <vector>

namespace odbc {

class KeywordDB;

class ODBC_PUBLIC_API KeywordMatcher
{
public:
    struct MatchResult
    {
        int matchedLength;
        bool found;
    };

    void updateFromDB(const KeywordDB* db);
    MatchResult findLongestKeywordMatching(const std::string& str) const;
    int longestKeywordLength() const;
    int longestKeywordWordCount() const;

private:
    std::vector<std::string> keywords_;
    int longestKeywordLength_ = 0;
    int longestKeywordWordCount_ = 0;
};

}
