#pragma once

#include "odbc/config.hpp"

namespace odbc {

char* newCStr(const char* str);
char* newCStrRange(const char* src, int beg, int end);
void deleteCStr(char* str);

}
