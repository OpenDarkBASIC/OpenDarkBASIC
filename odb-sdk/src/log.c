#include "odb-sdk/cli_colors.h"
#include "odb-sdk/config.h"
#include "odb-sdk/log.h"
#include <stdio.h>
#include <string.h>

static char progress_active;

/* ------------------------------------------------------------------------- */
#if defined(ODBSDK_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
static void
log_last_error_win32(FILE* fp)
{
    char* error;
    if (FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
                | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR)&error,
            0,
            NULL)
        != 0)
    {
        fprintf(fp, "(Failed to get error from FormatMessage())");
        return;
    }

    fprintf(fp, "%s", error);
    LocalFree(error);
}
#endif

/* ------------------------------------------------------------------------- */
static int
is_ascii_alpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
static int
is_ascii_numeric(char c)
{
    return c >= '0' && c <= '9';
}

/* ------------------------------------------------------------------------- */
struct varef
{
    va_list ap;
};
static const char*
process_standard_format(FILE* fp, const char* fmt, struct varef* args)
{
    int  i = 0;
    char subfmt[16];
    subfmt[i++] = *fmt++;
    do
    {
        subfmt[i++] = *fmt++;
    } while (i != 15 && (!is_ascii_alpha(fmt[-1]) || fmt[-1] == 'l')
             && fmt[-1] != '%');

    subfmt[i] = '\0';
    vfprintf(fp, subfmt, args->ap);

    /* Have to advance to next argument */
    /* XXX: Does this work on all compilers? */
#if defined(_MSC_VER)
    (void)va_arg(args->ap, void*);
#endif

    return fmt;
}

/* ------------------------------------------------------------------------- */
static const char*
debug_style(void)
{
    return FG_YELLOW;
}
static const char*
info_style(void)
{
    return FGB_WHITE;
}
static const char*
note_style(void)
{
    return FGB_MAGENTA;
}
static const char*
warn_style(void)
{
    return FGB_YELLOW;
}
static const char*
err_style(void)
{
    return FGB_RED;
}
static const char*
emph_style(void)
{
    return FGB_WHITE;
}
static const char*
quote_style(void)
{
    return "`" FGB_WHITE;
}
static const char*
end_quote_style(void)
{
    return COL_RESET "'";
}
static const char*
reset_style(void)
{
    return COL_RESET;
}
static int
next_control_sequence(
    const char* fmt, int* i, const char** start, const char** end)
{
    if (!fmt[*i])
        return 0;

    switch (fmt[(*i)++])
    {
        case 'd':
            *start = debug_style();
            *end = reset_style();
            return 1;
        case 'i':
            *start = info_style();
            *end = reset_style();
            return 1;
        case 'n':
            *start = note_style();
            *end = reset_style();
            return 1;
        case 'w':
            *start = warn_style();
            *end = reset_style();
            return 1;
        case 'e':
            if (memcmp(&fmt[*i], "mph", 3) == 0)
            {
                (*i) += 3;
                *start = emph_style();
                *end = reset_style();
                return 1;
            }
            *start = err_style();
            *end = reset_style();
            return 1;
        case 'q':
            if (memcmp(&fmt[*i], "uote", 4) == 0)
            {
                (*i) += 4;
                *start = quote_style();
                *end = end_quote_style();
                return 1;
            }
            break;
    }

    return 0;
}
static const char*
process_color_format(FILE* fp, const char* fmt, struct varef* args)
{
    int         i;
    const char* start;
    const char* end;
    const char* content;
    for (i = 0; next_control_sequence(fmt + 1, &i, &start, &end);)
    {
    }

    if (fmt[i] != ':')
    {
        putc(*fmt, fp);
        return fmt + 1;
    }

    for (i = 0; next_control_sequence(fmt + 1, &i, &start, &end);)
        fprintf(fp, "%s", start);
    content = fmt + i + 1;

    while (*content)
    {
        if (*content == '{')
            content = process_color_format(fp, content, args);
        else if (*content == '%')
            content = process_standard_format(fp, content, args);
        else if (*content == '}')
        {
            fprintf(fp, "%s", end);
            content++;
            break;
        }
        else
            putc(*content++, fp);
    }

    return content;
}

/* ------------------------------------------------------------------------- */
static void
vfprintf_with_color(FILE* fp, const char* fmt, struct varef* args)
{
    while (*fmt)
    {
        if (fmt[0] == '{')
            fmt = process_color_format(fp, fmt, args);
        else if (fmt[0] == '%')
            fmt = process_standard_format(fp, fmt, args);
        else
            putc(*fmt++, fp);
    }
}

static void
fprintf_with_color(FILE* fp, const char* fmt, ...)
{
    struct varef args;
    va_start(args.ap, fmt);
    vfprintf_with_color(fp, fmt, &args);
    va_end(args.ap);
}
/* ------------------------------------------------------------------------- */
void
log_vraw(const char* fmt, va_list ap)
{
    struct varef args;
    va_copy(args.ap, ap);
    vfprintf_with_color(stderr, fmt, &args);
}

/* ------------------------------------------------------------------------- */
void
log_vimpl(
    char        is_progress,
    const char* severity,
    const char* group,
    const char* fmt,
    va_list     ap)
{
    struct varef args;
    va_copy(args.ap, ap);

    if (progress_active)
    {
        fprintf(stderr, "\r\033[A\033[K");
        progress_active = 0;
    }
    if (is_progress)
        progress_active = 1;

    fprintf_with_color(stderr, group);
    fprintf_with_color(stderr, severity);
    vfprintf_with_color(stderr, fmt, &args);
}

/* ------------------------------------------------------------------------- */
void
log_vprogress(
    const char* group, int current, int total, const char* fmt, va_list ap)
{
    char buf[31];
    if (total > 0)
        sprintf(buf, "{i:[%d/%d]} ", current, total);
    else
        buf[0] = '\0';

    log_vimpl(1, buf, group, fmt, ap);
}
