#include "odbc/util/Str.hpp"
#include <string.h>
#include <stdlib.h>

namespace odbc {

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

}
