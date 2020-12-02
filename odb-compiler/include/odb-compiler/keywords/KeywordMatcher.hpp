#pragma once

#include "odb-compiler/config.hpp"
#include <string>
#include <string_view>
#include <vector>

namespace odb {

class KeywordIndex;

class KeywordMatcher
{
public:
    struct MatchResult
    {
        int matchedLength = 0;
        bool found = false;
    };

    ODBCOMPILER_PUBLIC_API void updateFromIndex(const KeywordIndex* db);
    ODBCOMPILER_PUBLIC_API MatchResult findLongestKeywordMatching(const std::string& str) const;
    ODBCOMPILER_PUBLIC_API int longestKeywordLength() const;
    ODBCOMPILER_PUBLIC_API int longestKeywordWordCount() const;

private:
    std::vector<std::string> keywords_;
    int longestKeywordLength_ = 0;
    int longestKeywordWordCount_ = 0;
};

}
