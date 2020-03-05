#pragma once

#include "odbc/config.hpp"
#include <string>
#include <string_view>
#include <vector>

namespace odbc {

class KeywordDB;

class KeywordMatcher
{
public:
    struct MatchResult
    {
        int matchedLength = 0;
        bool found = false;
    };

    ODBC_PUBLIC_API void updateFromDB(const KeywordDB* db);
    ODBC_PUBLIC_API MatchResult findLongestKeywordMatching(const std::string& str) const;
    ODBC_PUBLIC_API int longestKeywordLength() const;
    ODBC_PUBLIC_API int longestKeywordWordCount() const;

private:
    std::vector<std::string> keywords_;
    int longestKeywordLength_ = 0;
    int longestKeywordWordCount_ = 0;
};

}
