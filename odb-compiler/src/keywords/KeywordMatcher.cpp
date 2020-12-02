#include "odb-compiler/keywords/KeywordMatcher.hpp"
#include "odb-compiler/keywords/KeywordIndex.hpp"
#include "odb-util/Str.hpp"
#include <algorithm>
#include <iostream>
#include <cstring>

namespace odb {

// ----------------------------------------------------------------------------
static bool charIsSymbolToken(char c)
{
    if (c >= '0' && c <= '9')
        return true;
    if (c >= 'A' && c <= 'Z')
        return true;
    if (c >= 'a' && c <= 'z')
        return true;
    if (c == '_')
        return true;
    return false;
}

// ----------------------------------------------------------------------------
void KeywordMatcher::updateFromIndex(const KeywordIndex* db)
{
    longestKeywordLength_ = 0;
    longestKeywordWordCount_ = 0;
    keywords_ = db->keywordNamesAsList();

    // Keywords are case insensitive, so transform all to lower case
    for (auto& s : keywords_)
        std::transform(s.begin(), s.end(), s.begin(), [](char c){ return std::tolower(c); });

    // Lexicographic sort so we can binary search later
    std::sort(keywords_.begin(), keywords_.end(), [](const std::string& a,const  std::string& b) { return a < b; });

    // Find the longest keyword so the lexicographic comparison is faster during
    // binary search
    auto longestKeyword = std::max_element(keywords_.begin(), keywords_.end(),
            [](const std::string& a, const std::string& b) { return a.size() < b.size(); });
    if (longestKeyword != keywords_.end())
        longestKeywordLength_ = longestKeyword->size();

    // Find the maximum number of words that appear in a keyword. This is not
    // necessarily the longest keyword. Counting the number of spaces should
    // work in all cases.
    auto wordCount = [](const std::string& str) {
        return std::count(str.begin(), str.end(), ' ') + 1;
    };
    auto longestKeywordWordCount = std::max_element(keywords_.begin(), keywords_.end(),
            [&wordCount](const std::string& a, const std::string& b) {
                return wordCount(a) < wordCount(b);
            });
    if (longestKeywordWordCount != keywords_.end())
        longestKeywordWordCount_ = wordCount(*longestKeywordWordCount);
}

// ----------------------------------------------------------------------------
KeywordMatcher::MatchResult KeywordMatcher::findLongestKeywordMatching(const std::string& str) const
{
    auto first = keywords_.begin();
    auto last = keywords_.end();
    std::vector<std::vector<std::string>::const_iterator> keywordStack;
    keywordStack.reserve(longestKeywordWordCount());
    int matchedLen = 0;
    while (str[matchedLen])
    {
        auto compare = [&](const std::string& a, const std::string& b) {
            return std::tolower(a[matchedLen]) < std::tolower(b[matchedLen]);
        };
        first = std::lower_bound(first, last, str, compare);
        last = std::upper_bound(first, last, str, compare);
        if (first == last)
            break;

        ++matchedLen;
        if (!charIsSymbolToken(str[matchedLen]))
            keywordStack.push_back(first);
    }

    while (keywordStack.size())
    {
        auto keyword = keywordStack.back();
        if (keyword != keywords_.end() && strncicmp(str.c_str(), keyword->c_str(), keyword->length()) == 0)
            return { (int)keyword->length(), true };
        keywordStack.pop_back();
    }

    return { matchedLen, false };
}

// ----------------------------------------------------------------------------
int KeywordMatcher::longestKeywordLength() const
{
    return longestKeywordLength_;
}

// ----------------------------------------------------------------------------
int KeywordMatcher::longestKeywordWordCount() const
{
    return longestKeywordWordCount_;
}

}
