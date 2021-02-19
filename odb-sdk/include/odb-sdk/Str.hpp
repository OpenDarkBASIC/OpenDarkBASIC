#pragma once

#include "odb-sdk/config.hpp"
#include <string>
#include <vector>

namespace odb::str {

ODBSDK_PUBLIC_API char* newCStr(const char* str);
ODBSDK_PUBLIC_API char* newCStrRange(const char* src, size_t beg, size_t end);
ODBSDK_PUBLIC_API void deleteCStr(char* str);

ODBSDK_PUBLIC_API int strncicmp(const char* a, const char* b, size_t n);

ODBSDK_PUBLIC_API void replaceAll(std::string& subject,
                                  const std::string& search,
                                  const std::string& replace);

ODBSDK_PUBLIC_API std::string escapeBackslashes(const std::string& s);

ODBSDK_PUBLIC_API void toLowerInplace(std::string& str);
ODBSDK_PUBLIC_API std::string toLower(const std::string& str);

ODBSDK_PUBLIC_API void split(std::vector<std::string>* cont,
                             const std::string& str,
                             char delim = ' ');

ODBSDK_PUBLIC_API void justifyWrap(std::vector<std::string>* lines,
                                   const std::string& str,
                                   int width,
                                   char delim = ' ');

}
