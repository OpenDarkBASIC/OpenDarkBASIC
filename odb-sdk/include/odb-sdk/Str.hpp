#pragma once

#include "odb-sdk/config.hpp"
#include <string>

namespace odb {
namespace str {

ODBSDK_PUBLIC_API char* newCStr(const char* str);
ODBSDK_PUBLIC_API char* newCStrRange(const char* src, size_t beg, size_t end);
ODBSDK_PUBLIC_API void deleteCStr(char* str);

ODBSDK_PUBLIC_API int strncicmp(const char* a, const char* b, size_t n);

ODBSDK_PUBLIC_API void replaceAll(std::string& subject,
                                  const std::string& search,
                                  const std::string& replace);

ODBSDK_PUBLIC_API void toLowerInplace(std::string& str);
ODBSDK_PUBLIC_API std::string toLower(const std::string& str);

template <class Container>
void split(const std::string &str, Container &cont,
           char delim = ' ')
{
    std::size_t current, previous = 0;
    current = str.find(delim);
    while (current != std::string::npos)
    {
        cont.push_back(str.substr(previous, current - previous));
        previous = current + 1;
        current = str.find(delim, previous);
    }
    cont.push_back(str.substr(previous, current - previous));
}

}
}
