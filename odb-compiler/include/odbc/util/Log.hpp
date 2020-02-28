#pragma once

#include "odbc/config.hpp"
#include <cstdio>

namespace odbc {
namespace log {

enum Severity
{
    INFO,
    WARNING,
    ERROR,
    FATAL
};

void init();
void setInfoLog(FILE* fp);
void setDataLog(FILE* fp);

void info(const char* fmt, ...);
void data(const char* fmt, ...);

void dbParser(Severity severity, const char* fmt, ...);
void vdbParser(Severity severity, const char* fmt, va_list ap);
void kwParser(Severity severity, const char* fmt, ...);
void vkwParser(Severity severity, const char* fmt, va_list ap);
void ast(Severity severity, const char* fmt, ...);

}
}
