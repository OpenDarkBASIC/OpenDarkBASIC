#include "odb-sdk/Str.hpp"
#include <algorithm>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cassert>

namespace odb {
namespace str {

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
std::string escapeBackslashes(const std::string& s)
{
    std::string copy(s);
    size_t off = 0;
    for (size_t i = 0; i < s.length(); ++i)
        if (s[i] == '\\')
        {
            copy.insert(copy.begin() + i + off, '\\');
            off++;
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

}
}
