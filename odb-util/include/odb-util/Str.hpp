#pragma once

#include "odb-util/config.hpp"

namespace odb {

char* newCStr(const char* str);
char* newCStrRange(const char* src, int beg, int end);
void deleteCStr(char* str);

int strncicmp(const char* a, const char* b, int n);

}
