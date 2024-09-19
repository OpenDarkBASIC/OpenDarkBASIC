#include "odb-util/cli_colors.h"
#include "odb-util/config.h"
#include "odb-util/log.h"
#include "odb-util/thread.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static char                 progress_active;
static struct log_interface g_log;
static struct mutex* g_mutex;

/* -------------------------------------------------------------------------- */
static void
default_write_func(const char* fmt, va_list ap)
{
    vfprintf(stderr, fmt, ap);
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
           : g_log.use_color == 2 ? "{emph1}"
                                  : "";
}
static const char*
emph2_style(void)
{
    return g_log.use_color == 1   ? FGB_CYAN
           : g_log.use_color == 2 ? "{emph2}"
                                  : "";
}
static const char*
emph3_style(void)
{
    return g_log.use_color == 1   ? FGB_YELLOW
           : g_log.use_color == 2 ? "{emph3}"
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
            if (memcmp(&fmt[*i], "mph1", 4) == 0)
            {
                (*i) += 4;
                *start = emph1_style();
                *end = reset_style();
                return 1;
            }
            if (memcmp(&fmt[*i], "mph2", 4) == 0)
            {
                (*i) += 4;
                *start = emph2_style();
                *end = reset_style();
                return 1;
            }
            if (memcmp(&fmt[*i], "mph3", 4) == 0)
            {
                (*i) += 4;
                *start = emph3_style();
                *end = reset_style();
                return 1;
            }
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
log_vimpl(
    char        is_progress,
    const char* severity,
    const char* group,
    const char* fmt,
    va_list     ap)
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

    fprintf_with_color(group);
    fprintf_with_color(severity);
    vfprintf_with_color(fmt, &args);
    
    mutex_unlock(g_mutex);
}

/* -------------------------------------------------------------------------- */
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

/* -------------------------------------------------------------------------- */
void
log_vflc(
    const char*      severity,
    const char*      filename,
    const char*      source,
    struct utf8_span location,
    const char*      fmt,
    va_list          ap)
{
    utf8_idx     i;
    utf8_idx     l1, c1;
    struct varef args;
    
    mutex_lock(g_mutex);

    l1 = 1, c1 = 1;
    for (i = 0; i != location.off; i++)
    {
        c1++;
        if (source[i] == '\n')
            l1++, c1 = 1;
    }

    fprintf_with_color("{emph:%s:%d:%d:}", filename, l1, c1);
    if (*severity)
    {
        log_putc(' ');
        fprintf_with_color(severity);
    }

    va_copy(args.ap, ap);
    vfprintf_with_color(fmt, &args);
    
    mutex_unlock(g_mutex);
}

int
log_excerpt(const char* source, const struct log_highlight* highlights)
{
    utf8_idx         c, h;
    utf8_idx         l1, l2, line;
    utf8_idx         indent, max_indent, gutter_indent;
    int              num_highlights;
    struct utf8_span loc;
    struct utf8_span block;
    
    mutex_lock(g_mutex);

    ODBUTIL_DEBUG_ASSERT(
        highlights != NULL && highlights[0].new_text != NULL,
        log_util_err("Require at least one location"));

    /* Calculate union of all highlight locations, store into "loc" */
    ODBUTIL_DEBUG_ASSERT(highlights, (void)0);
    ODBUTIL_DEBUG_ASSERT(!LOG_IS_SENTINAL(highlights[0]), (void)0);
    loc.off = highlights[0].loc.off;
    loc.len = 0;
    for (h = 0; !LOG_IS_SENTINAL(highlights[h]); h++)
        if (highlights[h].type == LOG_HIGHLIGHT)
            loc = utf8_span_union(loc, highlights[h].loc);
    num_highlights = h;

    /* Calculate beginning of block and line number. The goal is to make
     * "block" point to the first character in the line that contains the
     * location. */
    l1 = 1, block.off = 0;
    for (c = 0; c != loc.off; c++)
        if (source[c] == '\n')
            l1++, block.off = c + 1;

    /* Calculate line of where the location ends */
    l2 = l1;
    for (c = 0; c != loc.len; c++)
        if (source[loc.off + c] == '\n')
            l2++;

    /* Find the end of the line for block */
    block = utf8_span_union(block, loc);
    block.len = loc.off - block.off + loc.len;
    for (;; block.len++, c++)
        if (source[loc.off + c] == '\n' || source[loc.off + c] == '\0')
        {
            block.len++; /* Include the newline */
            break;
        }

    /* Also keep track of the minimum indentation. This is used to unindent the
     * block of code as much as possible when printing out the excerpt. */
    max_indent = 10000;
    for (c = 0; c != block.len;)
    {
        indent = 0;
        for (; c != block.len; ++c, ++indent)
        {
            if (source[block.off + c] != ' ' && source[block.off + c] != '\t')
                break;
        }

        if (max_indent > indent)
            max_indent = indent;

        while (c != block.len)
            if (source[block.off + c++] == '\n')
                break;
    }

    /* Find width of the largest line number. This sets the indentation of the
     * gutter */
    gutter_indent = snprintf(NULL, 0, "%d", l2);
    gutter_indent += 2; /* Padding on either side of the line number */

    /* Main print loop */
    line = l1;
    for (c = 0; c != block.len; line++)
    {
        /* We will be looping over the same line multiple times, so must save
         * the first character and highlight */
        utf8_idx c_end;
        utf8_idx c_start = c;

        /* Print line of code ----------------------------------------------- */

        /* Gutter with line number and padding */
        log_printf("%*d | ", gutter_indent - 1, line);

        /* Next line may still be within a highlight. Enable style if so */
        for (h = 0; h != num_highlights; h++)
        {
            if (c >= highlights[h].loc.off - block.off
                && c < highlights[h].loc.off - block.off
                           + highlights[h].loc.len)
            {
                switch (highlights[h].type)
                {
                    case LOG_HIGHLIGHT:
                        log_printf("%s", emphn_style(highlights[h].group));
                        break;
                    case LOG_INSERT: break;
                    case LOG_REMOVE: break;
                }
            }
        }

        indent = 0;
        for (; c != block.len; c++)
        {
            if (indent++ < max_indent)
                continue;

            /* Enable style */
            for (h = 0; h != num_highlights; h++)
                if (c == highlights[h].loc.off - block.off)
                    switch (highlights[h].type)
                    {
                        case LOG_HIGHLIGHT:
                            log_printf("%s", emphn_style(highlights[h].group));
                            break;
                        case LOG_INSERT:
                            log_printf(
                                "%s%s%s",
                                insert_style(),
                                highlights[h].new_text,
                                reset_style());
                            break;
                        case LOG_REMOVE: break;
                    }

            /* Reached end of line? */
            if (source[block.off + c] == '\n' || source[block.off + c] == '\0')
            {
                for (h = 0; h != num_highlights; h++)
                    if (c >= highlights[h].loc.off - block.off
                        && c < highlights[h].loc.off - block.off
                                   + highlights[h].loc.len)
                        switch (highlights[h].type)
                        {
                            case LOG_HIGHLIGHT:
                                log_printf("%s", reset_style());
                                break;
                            case LOG_INSERT: break;
                            case LOG_REMOVE: break;
                        }

                c++;
                break;
            }

            /* Print char */
            log_putc(source[block.off + c]);

            /* Finish style */
            for (h = 0; h != num_highlights; h++)
                if (c
                    == highlights[h].loc.off - block.off + highlights[h].loc.len
                           - 1)
                    switch (highlights[h].type)
                    {
                        case LOG_HIGHLIGHT:
                            log_printf("%s", reset_style());
                            break;
                        case LOG_INSERT: break;
                        case LOG_REMOVE: break;
                    }
        }
        log_putc('\n');
        c_end = c;

        /* Print highlights ------------------------------------------------- */

        c = c_start;

        /* Gutter, but no line number because this line is for diagnostics */
        log_printf("%*s | ", gutter_indent - 1, "");

        /* Next line may still be within a highlight. Enable style if so */
        for (h = 0; h != num_highlights; h++)
            if (c >= highlights[h].loc.off - block.off
                && c < highlights[h].loc.off - block.off
                           + highlights[h].loc.len)
                switch (highlights[h].type)
                {
                    case LOG_HIGHLIGHT:
                        log_printf("%s", emphn_style(highlights[h].group));
                        break;
                    case LOG_INSERT: log_printf("%s", insert_style()); break;
                    case LOG_REMOVE: break;
                }

        indent = 0;
        for (c = c_start; c != c_end; ++c)
        {
            if (indent++ < max_indent)
                continue;

            /* Reached end of line? */
            if (source[block.off + c] == '\n' || source[block.off + c] == '\0')
            {
                for (h = 0; h != num_highlights; h++)
                    if (c >= highlights[h].loc.off - block.off
                        && c < highlights[h].loc.off - block.off
                                   + highlights[h].loc.len)
                        switch (highlights[h].type)
                        {
                            case LOG_HIGHLIGHT:
                                log_printf("%s", reset_style());
                                break;
                            case LOG_INSERT: goto cant_break_while_inserting;
                            case LOG_REMOVE: break;
                        }

                c++;
                break;
            }
        cant_break_while_inserting:;

            /* Print char */
            for (h = 0; h != num_highlights; ++h)
                switch (highlights[h].type)
                {
                    case LOG_HIGHLIGHT:
                        if (c == highlights[h].loc.off - block.off)
                            log_printf(
                                "%s%c",
                                emphn_style(highlights[h].group),
                                highlights[h].marker[0]);
                        else if (
                            c > highlights[h].loc.off - block.off
                            && c < highlights[h].loc.off - block.off
                                       + highlights[h].loc.len - 1)
                            log_putc(highlights[h].marker[1]);
                        else if (
                            c
                            == highlights[h].loc.off - block.off
                                   + highlights[h].loc.len - 1)
                            log_putc(highlights[h].marker[2]);
                        if (c
                            == highlights[h].loc.off - block.off
                                   + highlights[h].loc.len - 1)
                            log_printf("%s", reset_style());
                        break;
                    case LOG_INSERT: {
                        int ins_off;
                        if (c != highlights[h].loc.off - block.off)
                            break;
                        for (ins_off = 0; ins_off != highlights[h].loc.len;
                             ++ins_off)
                        {
                            if (c + ins_off
                                == highlights[h].loc.off - block.off)
                                log_printf(
                                    "%s%c",
                                    insert_style(),
                                    highlights[h].marker[0]);
                            else if (
                                c + ins_off > highlights[h].loc.off - block.off
                                && c + ins_off
                                       < highlights[h].loc.off - block.off
                                             + highlights[h].loc.len - 1)
                                log_putc(highlights[h].marker[1]);
                            else if (
                                c + ins_off
                                == highlights[h].loc.off - block.off
                                       + highlights[h].loc.len - 1)
                                log_putc(highlights[h].marker[2]);
                            if (c + ins_off
                                == highlights[h].loc.off - block.off
                                       + highlights[h].loc.len - 1)
                                log_printf("%s", reset_style());
                        }
                    }
                    break;
                    case LOG_REMOVE: break;
                }

            /* Scan ahead to see if the next highlight is on this line.
             * If not, we exit early so we don't add additional spaces
             */
            int c2;
            for (h = 0; h != num_highlights; ++h)
                if ((highlights[h].type == LOG_HIGHLIGHT
                     && c < highlights[h].loc.off - block.off
                                + highlights[h].loc.len)
                    || (highlights[h].type == LOG_INSERT
                        && c < highlights[h].loc.off - block.off))
                {
                    break;
                }
            for (c2 = c + 1; c2 != c_end; ++c2)
                if (source[block.off + c2] == '\n')
                    break;
            if (h == num_highlights || c2 < highlights[h].loc.off - block.off)
                break;

            if (c < highlights[h].loc.off - block.off)
                log_putc(' ');
        }
        /* DON'T print newline because annotations can be on the same line */

        /* Print annotations ------------------------------------------------ */

        for (h = num_highlights - 1; h >= 0; h--)
        {
            int h2;
            int num_proceeding;

            if (highlights[h].loc.off - block.off < c_start
                || highlights[h].loc.off - block.off >= c_end
                || !*highlights[h].annotation)
            {
                continue;
            }

            /* If this is the last annotation on the line, and the highlight
             * does not span over to the next line, it can be appended to the
             * current line without adding an extra gutter */
            num_proceeding = 0;
            for (h2 = h + 1; h2 != num_highlights; h2++)
                if (highlights[h2].loc.off - block.off >= c_start
                    && highlights[h2].loc.off - block.off <= c_end)
                    num_proceeding++;
            if (num_proceeding == 0)
            {
                log_printf(
                    " %s%s%s",
                    emphn_style(highlights[h].group),
                    highlights[h].annotation,
                    reset_style());
                continue;
            }

            /* Gutter without line number because this line is for diagnostics*/
            log_printf("\n%*s | ", gutter_indent - 1, "");

            for (indent = 0, c = c_start; c != c_end; c++)
            {
                if (indent++ < max_indent)
                    continue;

                if (c == highlights[h].loc.off - block.off)
                {
                    log_printf(
                        "%s%s%s",
                        emphn_style(highlights[h].group),
                        highlights[h].annotation,
                        reset_style());
                    break;
                }

                for (h2 = 0; h2 != num_highlights; ++h2)
                    if (c == highlights[h2].loc.off - block.off)
                    {
                        log_printf(
                            "%s|%s",
                            emphn_style(highlights[h2].group),
                            reset_style());
                        break;
                    }
                if (h2 == num_highlights)
                    log_putc(' ');
            }
        }
        log_putc('\n');
        c = c_end;
    }
    
    mutex_unlock(g_mutex);

    return gutter_indent;
}

void
log_excerpt_vimpl(
    int gutter_indent, const char* severity, const char* fmt, va_list ap)
{
    struct varef args;
    va_copy(args.ap, ap);

    mutex_lock(g_mutex);
    log_printf("%*s = ", gutter_indent - 1, "");
    fprintf_with_color(severity);
    vfprintf_with_color(fmt, &args);
    mutex_unlock(g_mutex);
}
