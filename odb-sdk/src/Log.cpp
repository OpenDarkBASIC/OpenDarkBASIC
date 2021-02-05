#include "odb-sdk/Log.hpp"
#include <cstdarg>
#include <cassert>

namespace odb {

// ----------------------------------------------------------------------------
Log::Log(FILE* stream) :
    stream_(stream),
    color_(RESET)
{
}

// ----------------------------------------------------------------------------
Log::~Log()
{
}

// ----------------------------------------------------------------------------
FILE* Log::getStream() const
{
    return stream_;
}

// ----------------------------------------------------------------------------
int Log::putc(char c)
{
    return ::putc(c, stream_);
}

// ----------------------------------------------------------------------------
int Log::print(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int result = vprint(fmt, ap);
    va_end(ap);

    return result;
}

// ----------------------------------------------------------------------------
int Log::print(Color color, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int result = vprint(color, fmt, ap);
    va_end(ap);

    return result;
}

// ----------------------------------------------------------------------------
int Log::vprint(const char* fmt, va_list ap)
{
    assert(stream_);
    return vfprintf(stream_, fmt, ap);
}

// ----------------------------------------------------------------------------
int Log::vprint(Color color, const char* fmt, va_list ap)
{
    assert(stream_);
    ColorState state(*this, color);
    return vfprintf(stream_, fmt, ap);
}

// ----------------------------------------------------------------------------
int Log::dbParser(Severity severity, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int result = vdbParser(severity, fmt, ap);
    va_end(ap);

    return result;
}

// ----------------------------------------------------------------------------
int Log::vdbParser(Severity severity, const char* fmt, va_list ap)
{
    return vlogPrefixSeverity("[db parser] ", severity, fmt, ap);
}

// ----------------------------------------------------------------------------
int Log::cmd(Severity severity, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int result = vcmd(severity, fmt, ap);
    va_end(ap);

    return result;
}

// ----------------------------------------------------------------------------
int Log::vcmd(Severity severity, const char* fmt, va_list ap)
{
    return vlogPrefixSeverity("[cmd] ", severity, fmt, ap);
}

// ----------------------------------------------------------------------------
int Log::ast(Severity severity, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int result = vlogPrefixSeverity("[ast] ", severity, fmt, ap);
    va_end(ap);

    return result;
}

// ----------------------------------------------------------------------------
int Log::sdk(Severity severity, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int result = vsdk(severity, fmt, ap);
    va_end(ap);

    return result;
}

// ----------------------------------------------------------------------------
int Log::vsdk(Severity severity, const char* fmt, va_list ap)
{
    return vlogPrefixSeverity("[sdk] ", severity, fmt, ap);
}

// ----------------------------------------------------------------------------
int Log::codegen(Severity severity, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int result = vsdk(severity, fmt, ap);
    va_end(ap);

    return result;
}

// ----------------------------------------------------------------------------
int Log::vcodegen(Severity severity, const char* fmt, va_list ap)
{
    return vlogPrefixSeverity("[codegen] ", severity, fmt, ap);
}

// ----------------------------------------------------------------------------
int Log::vlogPrefixSeverity(const char* prefix, Log::Severity severity, const char* fmt, va_list ap)
{
    int result = 0;

    switch (severity)
    {
        case Log::INFO : {
            result += info.print("%s", prefix);
            result += info.vprint(fmt, ap);
        } break;

        case Log::NOTICE :
        case Log::WARNING : {
            ColorState state(Log::info, FG_YELLOW);
            result += info.print("%s", prefix);
            result += info.vprint(fmt, ap);
        } break;

        case Log::ERROR :
        case Log::FATAL : {
            ColorState state(Log::info, FG_RED);
            result += info.print("%s", prefix);
            result += info.vprint(fmt, ap);
        } break;
    }

    return result;
}

// ----------------------------------------------------------------------------
Log Log::info(stderr);
Log Log::data(stdout);

// ----------------------------------------------------------------------------
ColorState::ColorState(Log& log, Log::Color color) :
    log_(log), saveColor_(log.color_)
{
    apply(log_, color);
}

// ----------------------------------------------------------------------------
ColorState::~ColorState()
{
    apply(log_, saveColor_);
}

// ----------------------------------------------------------------------------
void ColorState::apply(Log& log, Log::Color color)
{
    log.color_ = color;
    switch (color)
    {
        case Log::RESET              : log.print("\u001b[0m"); break;

        case Log::FG_BLACK           : log.print("\u001b[22;30m"); break;
        case Log::FG_RED             : log.print("\u001b[22;31m"); break;
        case Log::FG_GREEN           : log.print("\u001b[22;32m"); break;
        case Log::FG_YELLOW          : log.print("\u001b[22;33m"); break;
        case Log::FG_BLUE            : log.print("\u001b[22;34m"); break;
        case Log::FG_MAGENTA         : log.print("\u001b[22;35m"); break;
        case Log::FG_CYAN            : log.print("\u001b[22;36m"); break;
        case Log::FG_WHITE           : log.print("\u001b[22;37m"); break;

        case Log::BG_BLACK           : log.print("\u001b[22;40m"); break;
        case Log::BG_RED             : log.print("\u001b[22;41m"); break;
        case Log::BG_GREEN           : log.print("\u001b[22;42m"); break;
        case Log::BG_YELLOW          : log.print("\u001b[22;43m"); break;
        case Log::BG_BLUE            : log.print("\u001b[22;44m"); break;
        case Log::BG_MAGENTA         : log.print("\u001b[22;45m"); break;
        case Log::BG_CYAN            : log.print("\u001b[22;46m"); break;
        case Log::BG_WHITE           : log.print("\u001b[22;47m"); break;

        case Log::FG_BRIGHT_BLACK    : log.print("\u001b[1;30m"); break;
        case Log::FG_BRIGHT_RED      : log.print("\u001b[1;31m"); break;
        case Log::FG_BRIGHT_GREEN    : log.print("\u001b[1;32m"); break;
        case Log::FG_BRIGHT_YELLOW   : log.print("\u001b[1;33m"); break;
        case Log::FG_BRIGHT_BLUE     : log.print("\u001b[1;34m"); break;
        case Log::FG_BRIGHT_MAGENTA  : log.print("\u001b[1;35m"); break;
        case Log::FG_BRIGHT_CYAN     : log.print("\u001b[1;36m"); break;
        case Log::FG_BRIGHT_WHITE    : log.print("\u001b[1;37m"); break;

        case Log::BG_BRIGHT_BLACK    : log.print("\u001b[1;40m"); break;
        case Log::BG_BRIGHT_RED      : log.print("\u001b[1;41m"); break;
        case Log::BG_BRIGHT_GREEN    : log.print("\u001b[1;42m"); break;
        case Log::BG_BRIGHT_YELLOW   : log.print("\u001b[1;43m"); break;
        case Log::BG_BRIGHT_BLUE     : log.print("\u001b[1;44m"); break;
        case Log::BG_BRIGHT_MAGENTA  : log.print("\u001b[1;45m"); break;
        case Log::BG_BRIGHT_CYAN     : log.print("\u001b[1;46m"); break;
        case Log::BG_BRIGHT_WHITE    : log.print("\u001b[1;47m"); break;
    }
}

}
