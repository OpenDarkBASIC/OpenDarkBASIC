#include "odbc/parsers/keywords/KeywordMatcher.hpp"
#include "odbc/parsers/keywords/KeywordDB.hpp"
#include <algorithm>
#include <iostream>
#include <cstring>

namespace odbc {

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
void KeywordMatcher::updateFromDB(const KeywordDB* db)
{
    keywords_ = db->keywordsAsList();

    // Keywords are case insensitive, so transform all to lower case
    for (auto& s : keywords_)
        std::transform(s.begin(), s.end(), s.begin(), [](char c){ return std::tolower(c); });

    // Lexicographic sort so we can binary search later
    std::sort(keywords_.begin(), keywords_.end(), [](const std::string& a,const  std::string& b) { return a < b; });

    // Find the longest keyword so the lexicographic comparison is faster during
    // binary search
    auto longestKeyword = std::max_element(keywords_.begin(), keywords_.end(), [](const std::string& a, const std::string& b) { return a.size() < b.size(); });
    if (longestKeyword != keywords_.end())
        longestKeywordLength_ = longestKeyword->size();
    else
        longestKeywordLength_ = 0;
}

// ----------------------------------------------------------------------------
bool KeywordMatcher::findLongestKeywordMatching(const char* str, int* matchedLen) const
{
    auto first = keywords_.begin();
    auto last = keywords_.end();
    std::vector<std::vector<std::string>::const_iterator> keywordStack;
    keywordStack.reserve(6);
    *matchedLen = 0;
    while (str[*matchedLen])
    {
        auto compare = [&](const std::string& a, const std::string& b) {
            return std::tolower(a[*matchedLen]) < std::tolower(b[*matchedLen]);
        };
        first = std::lower_bound(first, last, str, compare);
        last = std::upper_bound(first, last, str, compare);
        if (first == last)
            break;

        ++*matchedLen;
        if (!charIsSymbolToken(str[*matchedLen]))
            keywordStack.push_back(first);
    }

    while (keywordStack.size())
    {
        auto keyword = keywordStack.back();
        if (keyword != keywords_.end() && strncmp(str, keyword->c_str(), keyword->length()) == 0)
        {
            *matchedLen = keyword->length();
            return true;
        }
        keywordStack.pop_back();
    }

    return false;
}

// ----------------------------------------------------------------------------
int KeywordMatcher::longestKeywordLength() const
{
    return longestKeywordLength_;
}

}
