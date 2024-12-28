#include "odb-util/cli_colors.h"
#include "odb-util/config.h"
#include "odb-util/log.h"
#include "odb-util/mutex.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

static char                 progress_active;
static struct log_interface g_log;
static struct mutex*        g_mutex;

/* -------------------------------------------------------------------------- */
static void
default_write_func(const char* fmt, va_list ap)
{
    vfprintf(stderr, fmt, ap);
    fflush(stderr);
}
ODBUTIL_PRINTF_FORMAT(1, 2)
static void
log_printf(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    g_log.write(fmt, ap);
    va_end(ap);
}
static void
log_putc(char c)
{
    log_printf("%c", c);
}

/* -------------------------------------------------------------------------- */
#if defined(ODBUTIL_PLATFORM_WINDOWS)
#else
#include <unistd.h>
#endif

static char
stream_is_terminal(FILE* fp)
{
#if defined(ODBUTIL_PLATFORM_WINDOWS)
    return 1;
#else
    return isatty(fileno(fp));
#endif
}

/* -------------------------------------------------------------------------- */
int
log_init(void)
{
    g_log.write = default_write_func;
    g_log.use_color = stream_is_terminal(stderr);
    g_mutex = mutex_create();

    if (g_mutex == NULL)
        return -1;
    return 0;
}

/* -------------------------------------------------------------------------- */
void
log_deinit(void)
{
    mutex_destroy(g_mutex);
}

/* -------------------------------------------------------------------------- */
struct log_interface
log_configure(struct log_interface iface)
{
    struct log_interface old = g_log;
    g_log = iface;
    return old;
}

/* -------------------------------------------------------------------------- */
int
log_has_color(void)
{
    return g_log.use_color;
}

/* -------------------------------------------------------------------------- */
#if defined(ODBUTIL_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
static void
log_last_error_win32(void)
{
    char* error;
    DWORD dwError = GetLastError();
    if (FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
                | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            dwError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR)&error,
            0,
            NULL)
        == 0)
    {
        log_printf("(Failed to get error from FormatMessage())");
        return;
    }

    log_printf("(%d) %s", dwError, error);
    LocalFree(error);
}
#endif

static void
log_last_error_posix(void)
{
    log_printf("(%d) %s", errno, strerror(errno));
}

/* -------------------------------------------------------------------------- */
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

/* -------------------------------------------------------------------------- */
struct varef
{
    va_list ap;
};
static const char*
process_standard_format(const char* fmt, struct varef* args)
{
    char subfmt[16];
    int  i = 0;
    char num_subargs = 1;
    subfmt[i++] = *fmt++;
    do
    {
        if (*fmt == '*')
            num_subargs++;
        subfmt[i++] = *fmt++;
    } while (i != 15 && (!is_ascii_alpha(fmt[-1]) || fmt[-1] == 'l')
             && fmt[-1] != '%');

    subfmt[i] = '\0';
    g_log.write(subfmt, args->ap);

    /* Have to advance to next argument */
    /* XXX: Does this work on all compilers? */
#if defined(_MSC_VER)
    while (num_subargs--)
        (void)va_arg(args->ap, void*);
#endif

    return fmt;
}

/* -------------------------------------------------------------------------- */
static const char*
help_style(void)
{
    return g_log.use_color == 1   ? FG_YELLOW
           : g_log.use_color == 2 ? "{help}"
                                  : "";
}
static const char*
debug_style(void)
{
    return g_log.use_color == 1   ? FG_YELLOW
           : g_log.use_color == 2 ? "{debug}"
                                  : "";
}
static const char*
info_style(void)
{
    return g_log.use_color == 1   ? FGB_WHITE
           : g_log.use_color == 2 ? "{info}"
                                  : "";
}
static const char*
note_style(void)
{
    return g_log.use_color == 1   ? FGB_MAGENTA
           : g_log.use_color == 2 ? "{note}"
                                  : "";
}
static const char*
warn_style(void)
{
    return g_log.use_color == 1   ? FGB_YELLOW
           : g_log.use_color == 2 ? "{warn}"
                                  : "";
}
static const char*
err_style(void)
{
    return g_log.use_color == 1 ? FGB_RED : g_log.use_color == 2 ? "{err}" : "";
}
static const char*
emph_style(void)
{
    return g_log.use_color == 1   ? FGB_WHITE
           : g_log.use_color == 2 ? "{emph}"
                                  : "";
}
static const char*
emph1_style(void)
{
    return g_log.use_color == 1   ? FGB_BLUE
           : g_log.use_color == 2 ? "{emph0}"
                                  : "";
}
static const char*
emph2_style(void)
{
    return g_log.use_color == 1   ? FGB_CYAN
           : g_log.use_color == 2 ? "{emph1}"
                                  : "";
}
static const char*
emph3_style(void)
{
    return g_log.use_color == 1   ? FGB_YELLOW
           : g_log.use_color == 2 ? "{emph2}"
                                  : "";
}
static const char*
emphn_style(int n)
{
    switch (n % 3)
    {
        case 0: return emph1_style();
        case 1: return emph2_style();
        case 2: return emph3_style();
    }
    return "";
}
static const char*
quote_style(void)
{
    return g_log.use_color == 1   ? "`" FGB_WHITE
           : g_log.use_color == 2 ? "{quote}"
                                  : "`";
}
static const char*
end_quote_style(void)
{
    return g_log.use_color == 1   ? COL_RESET "'"
           : g_log.use_color == 2 ? "{end_quote}"
                                  : "'";
}
static const char*
success_style(void)
{
    return g_log.use_color == 1   ? FGB_GREEN
           : g_log.use_color == 2 ? "{success}"
                                  : "";
}
static const char*
insert_style(void)
{
    return g_log.use_color == 1   ? FGB_GREEN
           : g_log.use_color == 2 ? "{insert}"
                                  : "";
}
static const char*
remove_style(void)
{
    return g_log.use_color == 1   ? FGB_RED
           : g_log.use_color == 2 ? "{remove}"
                                  : "";
}
static const char*
reset_style(void)
{
    return g_log.use_color == 1   ? COL_RESET
           : g_log.use_color == 2 ? "{reset}"
                                  : "";
}
static int
next_control_sequence(
    const char* fmt, int* i, const char** start, const char** end)
{
    if (!fmt[*i])
        return 0;

    switch (fmt[(*i)++])
    {
        case 'h':
            *start = help_style();
            *end = reset_style();
            return 1;
        case 'd':
            *start = debug_style();
            *end = reset_style();
            return 1;
        case 'i':
            if (memcmp(&fmt[*i], "nsert", 5) == 0)
            {
                (*i) += 5;
                *start = insert_style();
                *end = reset_style();
                return 1;
            }
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
                if (is_ascii_numeric(fmt[*i]))
                {
                    *start = emphn_style(fmt[*i] - '0');
                    *end = reset_style();
                    (*i)++;
                    return 1;
                }
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
        case 's':
            *start = success_style();
            *end = reset_style();
            return 1;
    }

    return 0;
}
static const char*
process_color_format(const char* fmt, struct varef* args)
{
    int         i;
    const char* content;
    const char* start = "";
    const char* end = "";

#if defined(ODBUTIL_PLATFORM_WINDOWS)
    if (memcmp(fmt, "{win32error}", 12) == 0)
    {
        log_last_error_win32();
        return fmt + 12;
    }
#endif
    if (memcmp(fmt, "{errno}", 7) == 0)
    {
        log_last_error_posix();
        return fmt + 7;
    }

    for (i = 0; next_control_sequence(fmt + 1, &i, &start, &end);)
    {
    }

    if (fmt[i] != ':')
    {
        log_putc(*fmt);
        return fmt + 1;
    }

    for (i = 0; next_control_sequence(fmt + 1, &i, &start, &end);)
        log_printf("%s", start);
    content = fmt + i + 1;

    while (*content)
    {
        if (*content == '{')
            content = process_color_format(content, args);
        else if (*content == '%')
            content = process_standard_format(content, args);
        else if (*content == '}')
        {
            log_printf("%s", end);
            content++;
            break;
        }
        else
            log_putc(*content++);
    }

    return content;
}

/* -------------------------------------------------------------------------- */
static void
vfprintf_with_color(const char* fmt, struct varef* args)
{
    while (*fmt)
    {
        if (fmt[0] == '{')
            fmt = process_color_format(fmt, args);
        else if (fmt[0] == '%')
            fmt = process_standard_format(fmt, args);
        else
            log_putc(*fmt++);
    }
}

static void
fprintf_with_color(const char* fmt, ...)
{
    struct varef args;
    va_start(args.ap, fmt);
    vfprintf_with_color(fmt, &args);
    va_end(args.ap);
}
/* -------------------------------------------------------------------------- */
void
log_vraw(const char* fmt, va_list ap)
{
    struct varef args;
    va_copy(args.ap, ap);
    mutex_lock(g_mutex);
    vfprintf_with_color(fmt, &args);
    mutex_unlock(g_mutex);
}

/* -------------------------------------------------------------------------- */
void
log_vimpl(char is_progress, const char* severity, const char* fmt, va_list ap)
{
    struct varef args;
    va_copy(args.ap, ap);

    mutex_lock(g_mutex);

    if (is_progress && !progress_active)
        log_printf("\n");
    if (progress_active)
    {
        if (g_log.use_color)
            log_printf("\r\033[A\033[K");
        progress_active = 0;
    }
    if (is_progress)
        progress_active = 1;

    fprintf_with_color(severity);
    vfprintf_with_color(fmt, &args);

    mutex_unlock(g_mutex);
}

/* -------------------------------------------------------------------------- */
void
log_vprogress(int current, int total, const char* fmt, va_list ap)
{
    char buf[31];
    if (total > 0)
        sprintf(buf, "{i:[%d/%d]} ", current, total);
    else
        buf[0] = '\0';

    log_vimpl(1, buf, fmt, ap);
}

