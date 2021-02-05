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
void Log::log(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vlog(fmt, ap);
    va_end(ap);
}

// ----------------------------------------------------------------------------
void Log::log(Color color, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vlog(color, fmt, ap);
    va_end(ap);
}

// ----------------------------------------------------------------------------
void Log::vlog(const char* fmt, va_list ap)
{
    assert(stream_);
    vfprintf(stream_, fmt, ap);
}

// ----------------------------------------------------------------------------
void Log::vlog(Color color, const char* fmt, va_list ap)
{
    assert(stream_);
    ColorState state(*this, color);
    vfprintf(stream_, fmt, ap);
}

// ----------------------------------------------------------------------------
void Log::dbParser(Severity severity, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vdbParser(severity, fmt, ap);
    va_end(ap);
}

// ----------------------------------------------------------------------------
void Log::vdbParser(Severity severity, const char* fmt, va_list ap)
{
    vlogPrefixSeverity("[db parser] ", severity, fmt, ap);
}

// ----------------------------------------------------------------------------
void Log::cmd(Severity severity, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vcmd(severity, fmt, ap);
    va_end(ap);
}

// ----------------------------------------------------------------------------
void Log::vcmd(Severity severity, const char* fmt, va_list ap)
{
    vlogPrefixSeverity("[cmd] ", severity, fmt, ap);
}

// ----------------------------------------------------------------------------
void Log::ast(Severity severity, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vlogPrefixSeverity("[ast] ", severity, fmt, ap);
    va_end(ap);
}

// ----------------------------------------------------------------------------
void Log::sdk(Severity severity, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsdk(severity, fmt, ap);
    va_end(ap);
}

// ----------------------------------------------------------------------------
void Log::vsdk(Severity severity, const char* fmt, va_list ap)
{
    vlogPrefixSeverity("[sdk] ", severity, fmt, ap);
}

// ----------------------------------------------------------------------------
void Log::vlogPrefixSeverity(const char* prefix, Log::Severity severity, const char* fmt, va_list ap)
{
    switch (severity)
    {
        case Log::INFO :
            break;

        case Log::NOTICE :
        case Log::WARNING : {
            ColorState state(Log::info, FG_YELLOW);
            info.log("%s", prefix);
            info.vlog(fmt, ap);
        } break;

        case Log::ERROR :
        case Log::FATAL : {
            ColorState state(Log::info, FG_RED);
            info.log("%s", prefix);
            info.vlog(fmt, ap);
        } break;
    }
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
        case Log::RESET              : log.log("\u001b[0m"); break;

        case Log::FG_BLACK           : log.log("\u001b[22;30m"); break;
        case Log::FG_RED             : log.log("\u001b[22;31m"); break;
        case Log::FG_GREEN           : log.log("\u001b[22;32m"); break;
        case Log::FG_YELLOW          : log.log("\u001b[22;33m"); break;
        case Log::FG_BLUE            : log.log("\u001b[22;34m"); break;
        case Log::FG_MAGENTA         : log.log("\u001b[22;35m"); break;
        case Log::FG_CYAN            : log.log("\u001b[22;36m"); break;
        case Log::FG_WHITE           : log.log("\u001b[22;37m"); break;

        case Log::BG_BLACK           : log.log("\u001b[22;40m"); break;
        case Log::BG_RED             : log.log("\u001b[22;41m"); break;
        case Log::BG_GREEN           : log.log("\u001b[22;42m"); break;
        case Log::BG_YELLOW          : log.log("\u001b[22;43m"); break;
        case Log::BG_BLUE            : log.log("\u001b[22;44m"); break;
        case Log::BG_MAGENTA         : log.log("\u001b[22;45m"); break;
        case Log::BG_CYAN            : log.log("\u001b[22;46m"); break;
        case Log::BG_WHITE           : log.log("\u001b[22;47m"); break;

        case Log::FG_BRIGHT_BLACK    : log.log("\u001b[1;30m"); break;
        case Log::FG_BRIGHT_RED      : log.log("\u001b[1;31m"); break;
        case Log::FG_BRIGHT_GREEN    : log.log("\u001b[1;32m"); break;
        case Log::FG_BRIGHT_YELLOW   : log.log("\u001b[1;33m"); break;
        case Log::FG_BRIGHT_BLUE     : log.log("\u001b[1;34m"); break;
        case Log::FG_BRIGHT_MAGENTA  : log.log("\u001b[1;35m"); break;
        case Log::FG_BRIGHT_CYAN     : log.log("\u001b[1;36m"); break;
        case Log::FG_BRIGHT_WHITE    : log.log("\u001b[1;37m"); break;

        case Log::BG_BRIGHT_BLACK    : log.log("\u001b[1;40m"); break;
        case Log::BG_BRIGHT_RED      : log.log("\u001b[1;41m"); break;
        case Log::BG_BRIGHT_GREEN    : log.log("\u001b[1;42m"); break;
        case Log::BG_BRIGHT_YELLOW   : log.log("\u001b[1;43m"); break;
        case Log::BG_BRIGHT_BLUE     : log.log("\u001b[1;44m"); break;
        case Log::BG_BRIGHT_MAGENTA  : log.log("\u001b[1;45m"); break;
        case Log::BG_BRIGHT_CYAN     : log.log("\u001b[1;46m"); break;
        case Log::BG_BRIGHT_WHITE    : log.log("\u001b[1;47m"); break;
    }
}

}
