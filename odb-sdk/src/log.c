#include "odb-sdk/cli_colors.h"
#include "odb-sdk/config.h"
#include "odb-sdk/log.h"
#include <stdio.h>
#include <string.h>

#if defined(ODBSDK_PLATFORM_WINDOWS)
#else
#include <unistd.h>
#endif

static char           progress_active;
static char           use_color;
static log_write_func write_func;

/* ------------------------------------------------------------------------- */
static int
stream_is_terminal(FILE* fp)
{
#if defined(ODBSDK_PLATFORM_WINDOWS)
    return 1;
#else
    return isatty(fileno(fp));
#endif
}

/* ------------------------------------------------------------------------- */
static void
default_write_func(const char* fmt, va_list ap)
{
    vfprintf(stderr, fmt, ap);
}

/* ------------------------------------------------------------------------- */
void
log_set_write_func(log_write_func func)
{
    write_func = func ? func : default_write_func;
}

/* ------------------------------------------------------------------------- */
void
log_set_color(char enable)
{
    use_color = enable;
}

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
        == 0)
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
underline_style(void)
{
    return FGB_BLUE;
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
        case 'u':
            *start = underline_style();
            *end = reset_style();
            return 1;
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

#if defined(ODBSDK_PLATFORM_WINDOWS)
    if (memcmp(fmt, "{win32error}", 12) == 0)
    {
        log_last_error_win32(fp);
        return fmt + 12;
    }
#endif

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

/* ------------------------------------------------------------------------- */
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

    l1 = 1, c1 = 1;
    for (i = 0; i != location.off; i++)
    {
        c1++;
        if (source[i] == '\n')
            l1++, c1 = 1;
    }

    fprintf_with_color(stderr, "{emph:%s:%d:%d:} ", filename, l1, c1);
    fprintf_with_color(stderr, severity);

    va_copy(args.ap, ap);
    vfprintf_with_color(stderr, fmt, &args);
}

/* ------------------------------------------------------------------------- */
void
log_excerpt(
    const char*      filename,
    const char*      source,
    struct utf8_span loc,
    const char*      highlight_text)
{
    utf8_idx         i;
    utf8_idx         l1, c1, l2, c2;
    utf8_idx         indent, max_indent;
    utf8_idx         gutter_indent;
    utf8_idx         line;
    struct utf8_span block;

    /* Calculate line column as well as beginning of block. The goal is to make
     * "block" point to the first character in the line that contains the
     * location. */
    l1 = 1, c1 = 1, block.off = 0;
    for (i = 0; i != loc.off; i++)
    {
        c1++;
        if (source[i] == '\n')
            l1++, c1 = 1, block.off = i + 1;
    }

    /* Calculate line/column of where the location ends */
    l2 = l1, c2 = c1;
    for (i = 0; i != loc.len; i++)
    {
        c2++;
        if (source[loc.off + i] == '\n')
            l2++, c2 = 1;
    }

    /* Find the end of the line for block */
    block.len = loc.off - block.off + loc.len;
    for (; source[loc.off + i]; block.len++, i++)
        if (source[loc.off + i] == '\n')
            break;

    /* We also keep track of the minimum indentation. This is used to unindent
     * the block of code as much as possible when printing out the excerpt. */
    max_indent = 10000;
    for (i = 0; i != block.len;)
    {
        indent = 0;
        for (; i != block.len; ++i, ++indent)
        {
            if (source[block.off + i] != ' ' && source[block.off + i] != '\t')
                break;
        }

        if (max_indent > indent)
            max_indent = indent;

        while (i != block.len)
            if (source[block.off + i++] == '\n')
                break;
    }

    /* Unindent columns */
    c1 -= max_indent;
    c2 -= max_indent;

    /* Find width of the largest line number. This sets the indentation of the
     * gutter */
    gutter_indent = snprintf(NULL, 0, "%d", l2);
    gutter_indent += 2; /* Padding on either side of the line number */

    /* Print line number, gutter, and block of code */
    line = l1;
    for (i = 0; i != block.len;)
    {
        fprintf(stderr, "%*d | ", gutter_indent - 1, line);

        if (i >= loc.off - block.off && i <= loc.off - block.off + loc.len)
            fprintf(stderr, "%s", underline_style());

        indent = 0;
        while (i != block.len)
        {
            if (i == loc.off - block.off)
                fprintf(stderr, "%s", underline_style());
            if (i == loc.off - block.off + loc.len)
                fprintf(stderr, "%s", reset_style());

            if (indent++ >= max_indent)
                putc(source[block.off + i], stderr);

            if (source[block.off + i++] == '\n')
            {
                if (i >= loc.off - block.off
                    && i <= loc.off - block.off + loc.len)
                    fprintf(stderr, "%s", reset_style());
                break;
            }
        }
        line++;
    }
    fprintf(stderr, "%s\n", reset_style());

    /* print underline */
    if (c2 > c1)
    {
        fprintf(stderr, "%*s|%*s", gutter_indent, "", c1, "");
        fprintf(stderr, "%s", underline_style());
        putc('^', stderr);
        for (i = c1 + 1; i < c2; ++i)
            putc('~', stderr);
        fprintf(stderr, "%s ", reset_style());
    }
    else
    {
        utf8_idx col, max_col;

        fprintf(stderr, "%*s| ", gutter_indent, "");
        fprintf(stderr, "%s", underline_style());
        for (i = 1; i < c2; ++i)
            putc('~', stderr);
        for (; i < c1; ++i)
            putc(' ', stderr);
        putc('^', stderr);

        /* Have to find length of the longest line */
        col = 1, max_col = 1;
        for (i = 0; i != block.len; ++i)
        {
            if (max_col < col)
                max_col = col;
            col++;
            if (source[block.off + i] == '\n')
                col = 1;
        }
        max_col -= max_indent;

        for (i = c1 + 1; i < max_col; ++i)
            putc('~', stderr);
        fprintf(stderr, "%s ", reset_style());
    }

    fprintf(stderr, "%s", highlight_text);
    putc('\n', stderr);
}

/* ------------------------------------------------------------------------- */
void
log_binop_excerpt(
    const char*      filename,
    const char*      source,
    struct utf8_span lhs,
    struct utf8_span op,
    struct utf8_span rhs,
    const char*      lhs_text,
    const char*      rhs_text)
{
    utf8_idx         i;
    utf8_idx         l1, c1, l2, c2;
    utf8_idx         indent, max_indent;
    utf8_idx         gutter_indent;
    utf8_idx         line;
    struct utf8_span loc;
    struct utf8_span block;

    /* Merge locations for proceeding calculations */
    loc.off = lhs.off;
    loc.len = rhs.off - lhs.off + rhs.len;

    /* Calculate line column as well as beginning of block. The goal is to make
     * "block" point to the first character in the line that contains the
     * location. */
    l1 = 1, c1 = 1, block.off = 0;
    for (i = 0; i != loc.off; i++)
    {
        c1++;
        if (source[i] == '\n')
            l1++, c1 = 1, block.off = i + 1;
    }

    /* Calculate line/column of where the location ends */
    l2 = l1, c2 = c1;
    for (i = 0; i != loc.len; i++)
    {
        c2++;
        if (source[loc.off + i] == '\n')
            l2++, c2 = 1;
    }

    /* Find the end of the line for block */
    block.len = loc.off - block.off + loc.len;
    for (; source[loc.off + i]; block.len++, i++)
        if (source[loc.off + i] == '\n')
            break;

    /* We also keep track of the minimum indentation. This is used to unindent
     * the block of code as much as possible when printing out the excerpt. */
    max_indent = 10000;
    for (i = 0; i != block.len;)
    {
        indent = 0;
        for (; i != block.len; ++i, ++indent)
        {
            if (source[block.off + i] != ' ' && source[block.off + i] != '\t')
                break;
        }

        if (max_indent > indent)
            max_indent = indent;

        while (i != block.len)
            if (source[block.off + i++] == '\n')
                break;
    }

    /* Unindent columns */
    c1 -= max_indent;
    c2 -= max_indent;

    /* Find width of the largest line number. This sets the indentation of the
     * gutter */
    gutter_indent = snprintf(NULL, 0, "%d", l2);
    gutter_indent += 2; /* Padding on either side of the line number */

    /* Print line number, gutter, and block of code */
    line = l1;
    for (i = 0; i != block.len;)
    {
        fprintf(stderr, "%*d | ", gutter_indent - 1, line);

        if (i >= loc.off - block.off && i <= loc.off - block.off + loc.len)
            fprintf(stderr, "%s", underline_style());

        indent = 0;
        while (i != block.len)
        {
            if (i == loc.off - block.off)
                fprintf(stderr, "%s", underline_style());
            if (i == loc.off - block.off + loc.len)
                fprintf(stderr, "%s", reset_style());

            if (indent++ >= max_indent)
                putc(source[block.off + i], stderr);

            if (source[block.off + i++] == '\n')
            {
                if (i >= loc.off - block.off
                    && i <= loc.off - block.off + loc.len)
                    fprintf(stderr, "%s", reset_style());
                break;
            }
        }
        line++;
    }
    fprintf(stderr, "%s\n", reset_style());

    /* print underline */
    if (c2 > c1)
    {
        fprintf(stderr, "%*s|%*s", gutter_indent, "", c1, "");
        fprintf(stderr, "%s", underline_style());
        putc('^', stderr);
        for (i = c1 + 1; i < c2; ++i)
            putc('~', stderr);
        fprintf(stderr, "%s ", reset_style());
    }
    else
    {
        utf8_idx col, max_col;

        fprintf(stderr, "%*s| ", gutter_indent, "");
        fprintf(stderr, "%s", underline_style());
        for (i = 1; i < c2; ++i)
            putc('~', stderr);
        for (; i < c1; ++i)
            putc(' ', stderr);
        putc('^', stderr);

        /* Have to find length of the longest line */
        col = 1, max_col = 1;
        for (i = 0; i != block.len; ++i)
        {
            if (max_col < col)
                max_col = col;
            col++;
            if (source[block.off + i] == '\n')
                col = 1;
        }
        max_col -= max_indent;

        for (i = c1 + 1; i < max_col; ++i)
            putc('~', stderr);
        fprintf(stderr, "%s ", reset_style());
    }

    putc('\n', stderr);
}
