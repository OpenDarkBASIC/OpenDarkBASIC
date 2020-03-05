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

ODBC_PUBLIC_API void init();
ODBC_PUBLIC_API void setInfoLog(FILE* fp);
ODBC_PUBLIC_API void setDataLog(FILE* fp);

ODBC_PUBLIC_API void info(const char* fmt, ...);
ODBC_PUBLIC_API void data(const char* fmt, ...);

ODBC_PUBLIC_API void dbParser(Severity severity, const char* fmt, ...);
ODBC_PUBLIC_API void vdbParser(Severity severity, const char* fmt, va_list ap);
ODBC_PUBLIC_API void kwParser(Severity severity, const char* fmt, ...);
ODBC_PUBLIC_API void vkwParser(Severity severity, const char* fmt, va_list ap);
ODBC_PUBLIC_API void ast(Severity severity, const char* fmt, ...);

}
}
