#include "odb-sdk/Str.hpp"
#include <algorithm>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cassert>

namespace odb::str {

// ----------------------------------------------------------------------------
char* newCStr(const char* str)
{
    size_t len = strlen(str);
    char* newStr = (char*)malloc(len + 1);
    if (newStr == nullptr)
        return nullptr;
    memcpy(newStr, str, len);
    newStr[len] = '\0';
    return newStr;
}

// ----------------------------------------------------------------------------
char* newCStrRange(const char* src, size_t beg, size_t end)
{
    assert(beg <= end);

    char* result = (char*)malloc(end - beg + 1);
    if (result == nullptr)
        return nullptr;
    strncpy(result, src + beg, end - beg);
    result[end - beg] = '\0';
    return result;
}

// ----------------------------------------------------------------------------
void deleteCStr(char* str)
{
    free(str);
}

// ----------------------------------------------------------------------------
int strncicmp(const char* a, const char* b, size_t n)
{
    for (;n; n--, a++, b++)
    {
        int d = std::tolower((unsigned char)*a) - std::tolower((unsigned char)*b);
        if (d || !*a)
            return d;
    }

    return 0;
}

// ----------------------------------------------------------------------------
void replaceAll(std::string& subject, const std::string& search, const std::string& replace)
{
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
}

// ----------------------------------------------------------------------------
std::string escape(const std::string& s)
{
    std::string copy(s);
    size_t off = 0;
    for (size_t i = 0; i < s.length(); ++i)
    {
        if (s[i] == '\\' || s[i] == '"')
        {
            copy.insert(copy.begin() + i + off, '\\');
            off++;
        }
    }

    return copy;
}

// ----------------------------------------------------------------------------
void toLowerInplace(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](char c){ return std::tolower(c); });
}

// ----------------------------------------------------------------------------
std::string toLower(const std::string& str)
{
    std::string s(str);
    toLowerInplace(s);
    return s;
}

// ----------------------------------------------------------------------------
void split(std::vector<std::string>* strlist,
           const std::string& str,
           char delim)
{
    std::size_t current, previous = 0;
    current = str.find(delim);
    while (current != std::string::npos)
    {
        strlist->push_back(str.substr(previous, current - previous));
        previous = current + 1;
        current = str.find(delim, previous);
    }
    strlist->push_back(str.substr(previous, current - previous));
}

// ----------------------------------------------------------------------------
void justifyWrap(std::vector<std::string>* lines,
                 const std::string& str,
                 int width,
                 char delim)
{
    std::vector<std::string> wordList;
    split(&wordList, str);
    std::vector<std::string>::const_iterator lineStart = wordList.begin();
    std::vector<std::string>::const_iterator lineEnd = wordList.begin();

    auto appendAndJustify = [&lines, &lineStart, &lineEnd]() {
        lines->push_back("");
        for (std::vector<std::string>::const_iterator it = lineStart; it != lineEnd; ++it)
        {
            if (it != lineStart)
                lines->back().append(" ");
            lines->back().append(*it);
        }
    };

    int len = 0;
    while (lineEnd != wordList.end())
    {
        // No space left for another word
        if (len + (int)lineEnd->length() > width)
        {
            appendAndJustify();
            lineStart = lineEnd;
            len = 0;
        }
        else
        {
            len += (int)lineEnd->length() + 1;  // +1 = space
            lineEnd++;
        }
    }

    if (lineStart != lineEnd)
        appendAndJustify();
}

}
