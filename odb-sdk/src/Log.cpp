#include "odb-sdk/Log.hpp"
#include <cstdarg>
#include <cassert>

namespace odb {
namespace log {

static FILE* infofp = nullptr;
static FILE* datafp = nullptr;

//static const char* WHITE = "\u001b[31m";
static const char* YELLOW = "\u001b[33m";
static const char* RED = "\u001b[31m";
static const char* RESET = "\u001b[0m";

// ----------------------------------------------------------------------------
void init()
{
    infofp = stderr;
    datafp = stdout;
}

// ----------------------------------------------------------------------------
void setInfoLog(FILE* fp)
{
    infofp = fp;
}

// ----------------------------------------------------------------------------
void info(const char* fmt, ...)
{
    assert(infofp);

    va_list ap;
    va_start(ap, fmt);
    vfprintf(infofp, fmt, ap);
    va_end(ap);
}

// ----------------------------------------------------------------------------
void data(const char* fmt, ...)
{
    assert(datafp);

    va_list ap;
    va_start(ap, fmt);
    vfprintf(datafp, fmt, ap);
    va_end(ap);
}

// ----------------------------------------------------------------------------
void dbParser(Severity severity, const char* fmt, ...)
{
    assert(infofp);

    va_list ap;
    va_start(ap, fmt);
    vdbParser(severity, fmt, ap);
    va_end(ap);
}
void vdbParser(Severity severity, const char* fmt, va_list ap)
{
    assert(infofp);

    switch (severity)
    {
        case INFO : break;
        case NOTICE :
        case WARNING : fprintf(infofp, "%s", YELLOW); break;
        case ERROR :
        case FATAL : fprintf(infofp, "%s", RED); break;
    }
    fprintf(infofp, "[db Parser] ");
    vfprintf(infofp, fmt, ap);
    fprintf(infofp, "%s\n", RESET);
}

// ----------------------------------------------------------------------------
void kwParser(Severity severity, const char* fmt, ...)
{
    assert(infofp);

    va_list ap;
    va_start(ap, fmt);
    vkwParser(severity, fmt, ap);
    va_end(ap);
}
void vkwParser(Severity severity, const char* fmt, va_list ap)
{
    assert(infofp);

    switch (severity)
    {
        case INFO : break;
        case NOTICE :
        case WARNING : fprintf(infofp, "%s", YELLOW); break;
        case ERROR :
        case FATAL : fprintf(infofp, "%s", RED); break;
    }
    fprintf(infofp, "[kw Parser] ");
    vfprintf(infofp, fmt, ap);
    fprintf(infofp, "%s\n", RESET);
}

// ----------------------------------------------------------------------------
void ast(Severity severity, const char* fmt, ...)
{
    assert(infofp);

    fprintf(infofp, "[ast] ");
    va_list ap;
    va_start(ap, fmt);
    vfprintf(infofp, fmt, ap);
    va_end(ap);
}

// ----------------------------------------------------------------------------
void sdk(Severity severity, const char* fmt, ...)
{
    assert(infofp);

    va_list ap;
    va_start(ap, fmt);
    vsdk(severity, fmt, ap);
    va_end(ap);
}
void vsdk(Severity severity, const char* fmt, va_list ap)
{
    assert(infofp);

    switch (severity)
    {
        case INFO : break;
        case NOTICE :
        case WARNING : fprintf(infofp, "%s", YELLOW); break;
        case ERROR :
        case FATAL : fprintf(infofp, "%s", RED); break;
    }
    fprintf(infofp, "[SDK] ");
    vfprintf(infofp, fmt, ap);
    fprintf(infofp, "%s\n", RESET);
}

}
}
