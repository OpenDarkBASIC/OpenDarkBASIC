#include "odbc/parsers/keywords/KeywordMatcher.hpp"
#include "odbc/parsers/keywords/KeywordDB.hpp"
#include <algorithm>
#include <iostream>

namespace odbc {

// ----------------------------------------------------------------------------
void KeywordMatcher::loadFromDB(const KeywordDB* db)
{
    keywords_ = db->keywordsAsList();
    for (auto& s : keywords_)
        std::transform(s.begin(), s.end(), s.begin(), [](char c){ return std::tolower(c); });
    std::sort(keywords_.begin(), keywords_.end(), [](const std::string& a,const  std::string& b) { return a < b; });
}

// ----------------------------------------------------------------------------
int KeywordMatcher::findLongestKeywordMatching(const char* str) const
{
    std::string s(str);
    std::transform(s.begin(), s.end(), s.begin(), [](char c){ return std::tolower(c); });
    auto lower = std::lower_bound(keywords_.begin(), keywords_.end(), s);

    if (lower == keywords_.end())
        return 0;

    // See how much the input string matches the keyword we found
    int len1 = 0;
    const char* found = lower->c_str();
    while (found[len1] == str[len1] && found[len1] != '\0')
        len1++;
    if (len1 == 0 && lower == keywords_.begin())
        return 0;

    // If str is longer than the actual keyword then lower_bound finds the
    // entry after, so check the previous keyword for a match
    lower--;
    found = lower->c_str();
    int len2 = 0;
    while (found[len2] == str[len2] && found[len2] != '\0')
        len2++;

    // choose longest
    int len = len1 > len2 ? len1 : len2;
    if (len == 0)
        return 0;

    return len;
}

}
