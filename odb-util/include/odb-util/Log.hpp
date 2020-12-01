#pragma once

#include "odb-util/config.hpp"
#include <cstdio>

namespace odb {
namespace log {

enum Severity
{
    INFO,
    WARNING,
    ERROR,
    FATAL
};

ODBUTIL_PUBLIC_API void init();
ODBUTIL_PUBLIC_API void setInfoLog(FILE* fp);
ODBUTIL_PUBLIC_API void setDataLog(FILE* fp);

ODBUTIL_PUBLIC_API void info(const char* fmt, ...);
ODBUTIL_PUBLIC_API void data(const char* fmt, ...);

ODBUTIL_PUBLIC_API void dbParser(Severity severity, const char* fmt, ...);
ODBUTIL_PUBLIC_API void vdbParser(Severity severity, const char* fmt, va_list ap);
ODBUTIL_PUBLIC_API void kwParser(Severity severity, const char* fmt, ...);
ODBUTIL_PUBLIC_API void vkwParser(Severity severity, const char* fmt, va_list ap);
ODBUTIL_PUBLIC_API void ast(Severity severity, const char* fmt, ...);

}
}
