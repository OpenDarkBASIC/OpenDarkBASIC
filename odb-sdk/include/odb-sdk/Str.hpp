#pragma once

#include "odb-sdk/config.hpp"

namespace odb {

ODBSDK_PUBLIC_API char* newCStr(const char* str);
ODBSDK_PUBLIC_API char* newCStrRange(const char* src, int beg, int end);
ODBSDK_PUBLIC_API void deleteCStr(char* str);

ODBSDK_PUBLIC_API int strncicmp(const char* a, const char* b, int n);

}
