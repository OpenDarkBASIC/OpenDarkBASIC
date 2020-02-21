#include "odbc/parsers/keywords/KeywordMatcher.hpp"
#include "odbc/parsers/keywords/KeywordDB.hpp"
#include <algorithm>
#include <iostream>
#include <cstring>

namespace odbc {

// ----------------------------------------------------------------------------
void KeywordMatcher::updateFromDB(const KeywordDB* db)
{
    keywords_ = db->keywordsAsList();
    for (auto& s : keywords_)
        std::transform(s.begin(), s.end(), s.begin(), [](char c){ return std::tolower(c); });
    std::sort(keywords_.begin(), keywords_.end(), [](const std::string& a,const  std::string& b) { return a < b; });
}

// ----------------------------------------------------------------------------
bool KeywordMatcher::findLongestKeywordMatching(const char* str, int* matchedLen) const
{
    *matchedLen = 0;
    bool keywordFound = false;

    // keywords are stored in lower case
    std::string s(str);
    std::transform(s.begin(), s.end(), s.begin(), [](char c){ return std::tolower(c); });

    // binary search for nearest keyword
    auto lower = std::lower_bound(keywords_.begin(), keywords_.end(), s);

    if (strncmp(str, "make", 4) == 0)
        puts(str);

    // See how much the input string matches the keyword we found
    int len1 = 0;
    if (lower != keywords_.end())
    {
        const char* found = lower->c_str();
        int keywordLen1 = strlen(found);
        while (found[len1] == str[len1] && found[len1] != '\0')
            len1++;

        // It must match exactly
        if (len1 == keywordLen1)
            keywordFound = true;
    }

    // If str is longer than the actual keyword then lower_bound finds the
    // entry after, so check the previous keyword for a match
    int len2 = 0;
    if (lower != keywords_.begin())
    {
        lower--;
        const char* found = lower->c_str();
        int keywordLen2 = strlen(found);
        while (found[len2] == str[len2] && found[len2] != '\0')
            len2++;

        // it must match exactly
        if (len2 == keywordLen2)
            keywordFound = true;
    }

    // choose longest
    *matchedLen = len1 > len2 ? len1 : len2;

    return keywordFound;
}

}
