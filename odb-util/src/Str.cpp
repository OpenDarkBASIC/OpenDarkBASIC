#include "odb-util/Str.hpp"
#include <algorithm>
#include <string>
#include <cstring>
#include <cstdlib>

namespace odb {

// ----------------------------------------------------------------------------
char* newCStr(const char* str)
{
    int len = strlen(str);
    char* newStr = (char*)malloc(len + 1);
    memcpy(newStr, str, len);
    newStr[len] = '\0';
    return newStr;
}

// ----------------------------------------------------------------------------
char* newCStrRange(const char* src, int beg, int end)
{
    char* result = (char*)malloc(end - beg + 1);
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
int strncicmp(const char* a, const char* b, int n)
{
    for (;n; n--, a++, b++)
    {
        int d = std::tolower((unsigned char)*a) - std::tolower((unsigned char)*b);
        if (d || !*a)
            return d;
    }

    return 0;
}

}
