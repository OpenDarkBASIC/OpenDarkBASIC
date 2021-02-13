#pragma once

#include "odb-sdk/config.hpp"
#include <cstdio>

namespace odb {

class ODBSDK_PUBLIC_API Log
{
public:
    enum Severity
    {
        INFO,
        NOTICE,
        WARNING,
        ERROR,
        FATAL
    };

    enum Color
    {
        RESET,

        FG_BLACK,
        FG_RED,
        FG_GREEN,
        FG_YELLOW,
        FG_BLUE,
        FG_MAGENTA,
        FG_CYAN,
        FG_WHITE,

        BG_BLACK,
        BG_RED,
        BG_GREEN,
        BG_YELLOW,
        BG_BLUE,
        BG_MAGENTA,
        BG_CYAN,
        BG_WHITE,

        FG_BRIGHT_BLACK,
        FG_BRIGHT_RED,
        FG_BRIGHT_GREEN,
        FG_BRIGHT_YELLOW,
        FG_BRIGHT_BLUE,
        FG_BRIGHT_MAGENTA,
        FG_BRIGHT_CYAN,
        FG_BRIGHT_WHITE,

        BG_BRIGHT_BLACK,
        BG_BRIGHT_RED,
        BG_BRIGHT_GREEN,
        BG_BRIGHT_YELLOW,
        BG_BRIGHT_BLUE,
        BG_BRIGHT_MAGENTA,
        BG_BRIGHT_CYAN,
        BG_BRIGHT_WHITE,
    };

    Log(FILE* fp);
    ~Log();

    FILE* getStream() const;
    int putc(char c);
    int print(const char* fmt, ...);
    int print(Color color, const char* fmt, ...);
    int vprint(const char* fmt, va_list ap);
    int vprint(Color color, const char* fmt, va_list ap);

    /*!
     * Returns the current color enabled state (true if enabled, false if disabled)
     * and enables colors.
     */
    bool enableColor();
    /*!
     * Returns the current color enabled state (true if enabled, false if disabled)
     * and disables colors.
     */
    bool disableColor();

    static void dbParserFailedToOpenFile(const char* fileName);
    static void dbParserNotice(const char* fmt, ...);
    static void dbParserError(const char* fmt, ...);
    static void dbParserSyntaxWarning(const char* fileLineColumn, const char* fmt, ...);
    static void dbParserSyntaxError(const char* fileLineColumn, const char* fmt, ...);
    static void dbParserSemanticError(const char* fileLineColumn, const char* fmt, ...);
    static void dbParserLocationNote(const char* fileLineColumn, const char* fmt, ...);
    static void vdbParserFatalError(const char* fileLineColumn, const char* fmt, va_list ap);

    static int cmd(Log::Severity severity, const char* fmt, ...);
    static int vcmd(Log::Severity severity, const char* fmt, va_list ap);
    static int ast(Log::Severity severity, const char* fmt, ...);
    static int sdk(Log::Severity severity, const char* fmt, ...);
    static int vsdk(Log::Severity severity, const char* fmt, va_list ap);
    static int vlogPrefixSeverity(const char* prefix, Log::Severity severity, const char* fmt, va_list ap);
    static int codegen(Log::Severity severity, const char* fmt, ...);
    static int vcodegen(Log::Severity severity, const char* fmt, va_list ap);

    static Log info;
    static Log data;

private:
    friend class ColorState;

    FILE* stream_ = nullptr;
    Color color_ = RESET;
    bool enableColor_ = true;
};

class ODBSDK_PUBLIC_API ColorState
{
public:
    ColorState(Log& log, Log::Color color);
    ~ColorState();

private:
    void apply(Log& log, Log::Color color);

private:
    Log& log_;
    Log::Color saveColor_;
};

}
