#include "odb-sdk/cli_colors.h"
#include "odb-sdk/config.h"
#include "odb-sdk/log.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static char                 progress_active;
static struct log_interface g_log;

/* -------------------------------------------------------------------------- */
static void
default_write_func(const char* fmt, va_list ap)
{
    vfprintf(stderr, fmt, ap);
}
ODBSDK_PRINTF_FORMAT(1, 2)
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
#if defined(ODBSDK_PLATFORM_WINDOWS)
#else
#include <unistd.h>
#endif

static char
stream_is_terminal(FILE* fp)
{
#if defined(ODBSDK_PLATFORM_WINDOWS)
    return 1;
#else
    return isatty(fileno(fp));
#endif
}

/* -------------------------------------------------------------------------- */
void
log_init(void)
{
    g_log.write = default_write_func;
    g_log.use_color = stream_is_terminal(stderr);
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
#if defined(ODBSDK_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
static void
log_last_error_win32(void)
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
        log_printf("(Failed to get error from FormatMessage())");
        return;
    }

    log_printf("%s", error);
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
    return g_log.use_color ? FG_YELLOW : "";
}
static const char*
debug_style(void)
{
    return g_log.use_color ? FG_YELLOW : "";
}
static const char*
info_style(void)
{
    return g_log.use_color ? FGB_WHITE : "";
}
static const char*
note_style(void)
{
    return g_log.use_color ? FGB_MAGENTA : "";
}
static const char*
warn_style(void)
{
    return g_log.use_color ? FGB_YELLOW : "";
}
static const char*
err_style(void)
{
    return g_log.use_color ? FGB_RED : "";
}
static const char*
emph_style(void)
{
    return g_log.use_color ? FGB_WHITE : "";
}
static const char*
emph1_style(void)
{
    return g_log.use_color ? FGB_BLUE : "";
}
static const char*
emph2_style(void)
{
    return g_log.use_color ? FGB_CYAN : "";
}
static const char*
emph3_style(void)
{
    return g_log.use_color ? FGB_YELLOW : "";
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
    return g_log.use_color ? "`" FGB_WHITE : "`";
}
static const char*
end_quote_style(void)
{
    return g_log.use_color ? COL_RESET "'" : "'";
}
static const char*
success_style(void)
{
    return g_log.use_color ? FGB_GREEN : "";
}
static const char*
insert_style(void)
{
    return g_log.use_color ? FGB_GREEN : "";
}
static const char*
remove_style(void)
{
    return g_log.use_color ? FGB_RED : "";
}
static const char*
reset_style(void)
{
    return g_log.use_color ? COL_RESET : "";
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

#if defined(ODBSDK_PLATFORM_WINDOWS)
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
    vfprintf_with_color(fmt, &args);
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
}

/* -------------------------------------------------------------------------- */
int
log_excerpt(const char* source, const struct log_excerpt_inst* inst)
{
    utf8_idx         i, j, r;
    utf8_idx         l1, l2, line;
    utf8_idx         indent, max_indent, gutter_indent;
    int              num_inst;
    struct utf8_span loc;
    struct utf8_span block;

    ODBSDK_DEBUG_ASSERT(
        inst != NULL && inst[0].new_text != NULL,
        log_sdk_err("Require at least one location"));

    /* Calculate union of all locations */
    loc = inst[0].loc;
    for (i = 1; inst[i].new_text; ++i)
    {
        ODBSDK_DEBUG_ASSERT(
            inst[i - 1].loc.off <= inst[i].loc.off,
            log_sdk_err(
                "i-1: %d, i: %d\n", inst[i - 1].loc.off, inst[i].loc.off));
        if (inst[i + 1].new_text == NULL)
        {
            loc.len = inst[i].loc.off - inst[0].loc.off;
            if (inst[i].type == LOG_EXCERPT_HIGHLIGHT)
                loc.len += inst[i].loc.len;
        }
    }
    num_inst = i;

    /* Calculate beginning of block and line number. The goal is to make
     * "block" point to the first character in the line that contains the
     * location. */
    l1 = 1, block.off = 0;
    for (r = 0; r != loc.off; r++)
        if (source[r] == '\n')
            l1++, block.off = r + 1;

    /* Calculate line of where the location ends */
    l2 = l1;
    for (r = 0; r != loc.len; r++)
        if (source[loc.off + r] == '\n')
            l2++;

    /* Find the end of the line for block */
    block.len = loc.off - block.off + loc.len;
    for (; source[loc.off + r]; block.len++, r++)
        if (source[loc.off + r] == '\n')
        {
            block.len++; /* Include the newline */
            break;
        }

    /* Also keep track of the minimum indentation. This is used to unindent the
     * block of code as much as possible when printing out the excerpt. */
    max_indent = 10000;
    for (r = 0; r != block.len;)
    {
        indent = 0;
        for (; r != block.len; ++r, ++indent)
        {
            if (source[block.off + r] != ' ' && source[block.off + r] != '\t')
                break;
        }

        if (max_indent > indent)
            max_indent = indent;

        while (r != block.len)
            if (source[block.off + r++] == '\n')
                break;
    }

    /* Find width of the largest line number. This sets the indentation of the
     * gutter */
    gutter_indent = snprintf(NULL, 0, "%d", l2);
    gutter_indent += 2; /* Padding on either side of the line number */

    /* Main print loop */
    line = l1;
    for (r = 0; r != block.len; line++)
    {
        utf8_idx r_end;
        utf8_idx r_start = r;
        utf8_idx r2 = r;

        /* Print line of code ----------------------------------------------- */

        /* Gutter with line number and padding */
        log_printf("%*d | ", gutter_indent - 1, line);

        /* Enable style, in case the previous line had one */
        for (i = 0; i != num_inst; i++)
        {
            if (r >= inst[i].loc.off - block.off
                && r < inst[i].loc.off - block.off + inst[i].loc.len)
                log_printf("%s", emphn_style(inst[i].highlight_group));
        }

        indent = 0;
        for (; r != block.len; r++)
        {
            /* Enable/disable style */
            for (i = 0; i != num_inst; i++)
            {
                if (r == inst[i].loc.off - block.off)
                    log_printf("%s", emphn_style(inst[i].highlight_group));
                if (r == inst[i].loc.off - block.off + inst[i].loc.len)
                    log_printf("%s", reset_style());
            }

            if (indent++ < max_indent)
                continue;

            for (i = 0; i != num_inst; i++)
            {
                if (inst[i].type == LOG_EXCERPT_INSERT
                    && r == inst[i].loc.off - block.off)
                {
                    log_printf(
                        "%s%s%s",
                        insert_style(),
                        inst[i].new_text,
                        reset_style());
                }
            }

            if (source[block.off + r] == '\n')
            {
                r++;
                break;
            }

            log_putc(source[block.off + r]);
        }
        log_printf("%s\n", reset_style());

        /* Exclude the newline */
        r_end = source[block.off + r - 1] == '\n' ? r - 1 : r;

        /* Insertions affect r_end */
        for (i = 0; i != num_inst; i++)
            if (inst[i].type == LOG_EXCERPT_INSERT)
                r_end += inst[i].loc.len;

        /* Print highlights ------------------------------------------------- */

        /* Gutter, but no line number because this line is for diagnostics */
        log_printf("%*s | ", gutter_indent - 1, "");

        /* Enable style, in case the previous line had one */
        for (i = 0; i != num_inst; i++)
        {
            if (r_start >= inst[i].loc.off - block.off
                && r_end < inst[i].loc.off - block.off + inst[i].loc.len)
                log_printf(
                    "%s",
                    inst[i].type == LOG_EXCERPT_INSERT
                        ? insert_style()
                        : emphn_style(inst[i].highlight_group));
        }

        int ins = 0;
        i = 0;
        for (indent = 0, r2 = r_start; r2 != r_end; r2++)
        {
            /* Enable/disable style */
            for (j = 0; j != num_inst; j++)
            {
                if (r2 == inst[j].loc.off - block.off + ins)
                    log_printf(
                        "%s",
                        inst[i].type == LOG_EXCERPT_INSERT
                            ? insert_style()
                            : emphn_style(inst[j].highlight_group));
                if (r2 == inst[j].loc.off + ins - block.off + inst[j].loc.len)
                    log_printf("%s", reset_style());
            }

            if (indent++ < max_indent)
                continue;

            for (j = i; j != num_inst; j++)
            {
                if (r2 == inst[j].loc.off + ins - block.off)
                    log_putc('^');
                else if (
                    r2 > inst[j].loc.off + ins - block.off
                    && r2 < inst[j].loc.off + ins - block.off + inst[j].loc.len
                                - 1)
                    log_putc('~');
                else if (
                    r2
                    == inst[j].loc.off + ins - block.off + inst[j].loc.len - 1)
                    log_putc('<');
                else
                    continue;

                if (r2
                    == inst[j].loc.off + ins - block.off + inst[j].loc.len - 1)
                {
                    i++;
                    if (inst[j].type == LOG_EXCERPT_INSERT)
                        ins += inst[j].loc.len;
                }

                break;
            }
            if (i == num_inst)
                break;
            if (j == num_inst
                && r2 < inst[j - 1].loc.off + ins - block.off
                            + inst[j - 1].loc.len)
                log_putc(' ');
        }

        /* Print annotations ------------------------------------------------ */

        for (i = num_inst - 1; i >= 0; i--)
        {
            if (inst[i].loc.off - block.off < r_start
                || inst[i].loc.off - block.off >= r_end || !*inst[i].annotation)
            {
                continue;
            }

            /* If this is the last annotation on the line, and the highlight
             * does not span over to the next line, it can be appended to the
             * current line without adding an extra gutter */
            int num_anns = 0;
            for (j = i; j != num_inst; j++)
                if (inst[j].loc.off - block.off >= r_start
                    && inst[i].loc.off - block.off + inst[i].loc.len <= r_end
                    && *inst[i].annotation)
                    num_anns++;
            if (num_anns == 1)
            {
                log_printf(
                    " %s%s%s",
                    emphn_style(inst[i].highlight_group),
                    inst[i].annotation,
                    reset_style());
                continue;
            }

            /* Gutter without line number because this line is for diagnostics*/
            log_printf("\n%*s | ", gutter_indent - 1, "");

            for (indent = 0, r2 = r_start; r2 != r_end; r2++)
            {
                if (indent++ < max_indent)
                    continue;

                if (r2 == inst[i].loc.off - block.off)
                {
                    log_printf(
                        "%s%s%s",
                        emphn_style(inst[i].highlight_group),
                        inst[i].annotation,
                        reset_style());
                    break;
                }

                for (j = 0; j != num_inst; ++j)
                    if (r2 == inst[j].loc.off - block.off)
                    {
                        log_printf(
                            "%s|%s",
                            emphn_style(inst[j].highlight_group),
                            reset_style());
                        break;
                    }
                if (j == num_inst)
                    log_putc(' ');
            }
        }

        log_printf("%s\n", reset_style());
    }

    return gutter_indent;
}

/* -------------------------------------------------------------------------- */
int
log_excerpt_binop(
    const char*      source,
    struct utf8_span lhs,
    struct utf8_span op,
    struct utf8_span rhs,
    const char*      lhs_text,
    const char*      rhs_text)
{
    utf8_idx         i;
    utf8_idx         l1, l2;
    utf8_idx         indent, max_indent;
    utf8_idx         gutter_indent;
    utf8_idx         line;
    struct utf8_span loc;
    struct utf8_span block;
    char             lhs_text_written = 0;
    char             postpone_lhs_text = 0;

    /* Calculate an overall location for LHS,op,RHS */
    loc.off = lhs.off;
    loc.len = rhs.off - lhs.off + rhs.len;

    /* Calculate beginning of block and line number. The goal is to make
     * "block" point to the first character in the line that contains
     * the location. */
    l1 = 1, block.off = 0;
    for (i = 0; i != loc.off; i++)
        if (source[i] == '\n')
            l1++, block.off = i + 1;

    /* Calculate line of where the location ends */
    l2 = l1;
    for (i = 0; i != loc.len; i++)
        if (source[loc.off + i] == '\n')
            l2++;

    /* Find the end of the line for block */
    block.len = loc.off - block.off + loc.len;
    for (; source[loc.off + i]; block.len++, i++)
        if (source[loc.off + i] == '\n')
            break;

    /* We also keep track of the minimum indentation. This is used to
     * unindent the block of code as much as possible when printing out
     * the excerpt. */
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

    /* Find width of the largest line number. This sets the indentation
     * of the gutter */
    gutter_indent = snprintf(NULL, 0, "%d", l2);
    gutter_indent += 2; /* Padding on either side of the line number */

    /* Main print loop */
    line = l1;
    for (i = 0; i != block.len; line++)
    {
        int j = i;

        /* Gutter with line number and padding */
        log_printf("%*d | ", gutter_indent - 1, line);
        if (i >= lhs.off - block.off && i < lhs.off - block.off + lhs.len)
            log_printf("%s", emph1_style());
        if (i >= op.off - block.off && i < op.off - block.off + op.len)
            log_printf("%s", emph3_style());
        if (i >= rhs.off - block.off && i < rhs.off - block.off + rhs.len)
            log_printf("%s", emph2_style());

        /* Print line of code */
        indent = 0;
        while (i != block.len)
        {
            if (i == lhs.off - block.off)
                log_printf("%s", emph1_style());
            if (i == lhs.off - block.off + lhs.len)
                log_printf("%s", reset_style());
            if (i == op.off - block.off)
                log_printf("%s", emph3_style());
            if (i == op.off - block.off + op.len)
                log_printf("%s", reset_style());
            if (i == rhs.off - block.off)
                log_printf("%s", emph2_style());
            if (i == rhs.off - block.off + rhs.len)
                log_printf("%s", reset_style());

            if (source[block.off + i++] == '\n')
                break;
            if (indent++ >= max_indent)
                log_putc(source[block.off + i - 1]);
        }
        log_printf("%s\n", reset_style());

        /* Gutter, but no line number because this line is for
         * diagnostics */
        log_printf("%*s | ", gutter_indent - 1, "");
        if (j >= lhs.off - block.off && j < lhs.off - block.off + lhs.len)
            log_printf("%s", emph1_style());
        if (j >= op.off - block.off && j < op.off - block.off + op.len)
            log_printf("%s", emph3_style());
        if (j >= rhs.off - block.off && j < rhs.off - block.off + rhs.len)
            log_printf("%s", emph2_style());

        /* Print diagnostics */
        for (indent = 0; j != block.len; j++)
        {
            if (source[block.off + j] == '\n')
                break;

            if (j == lhs.off - block.off)
                log_printf("%s", emph1_style());
            if (j == lhs.off - block.off + lhs.len)
                log_printf("%s", reset_style());
            if (j == op.off - block.off)
                log_printf("%s", emph3_style());
            if (j == op.off - block.off + op.len)
                log_printf("%s", reset_style());
            if (j == rhs.off - block.off)
                log_printf("%s", emph2_style());
            if (j == rhs.off - block.off + rhs.len)
                log_printf("%s", reset_style());

            if (indent < max_indent)
                indent++;
            else if (j == lhs.off - block.off)
                log_putc(lhs.len > 1 ? '>' : '^');
            else if (
                j > lhs.off - block.off && j < lhs.off - block.off + lhs.len)
                log_putc('~');
            else if (j >= op.off - block.off && j < op.off - block.off + op.len)
                log_putc('^');
            else if (
                j >= rhs.off - block.off
                && j < rhs.off - block.off + rhs.len - 1)
                log_putc('~');
            else if (j == rhs.off - block.off + rhs.len - 1)
            {
                log_putc(rhs.len > 1 ? '<' : '^');
                log_printf(" %s%s%s", emph2_style(), rhs_text, reset_style());
                break;
            }
            else
                log_putc(' ');
        }

        /* Print lhs text if appropriate. If lhs and rhs texts both
         * appear on the same line, then the lhs text it written later,
         * one line below */
        if (j >= rhs.off - block.off)
            postpone_lhs_text = 1;
        if (j >= lhs.off - block.off + lhs.len && !lhs_text_written
            && !postpone_lhs_text)
        {
            log_printf(" %s%s%s", emph1_style(), lhs_text, reset_style());
            lhs_text_written = 1;
        }

        log_printf("%s\n", reset_style());
    }

    if (!lhs_text_written && postpone_lhs_text && *lhs_text)
    {
        log_printf("%*s | ", gutter_indent - 1, "");
        for (i = 0; i != block.len;)
            for (indent = 0; i != block.len; indent++)
            {
                if (i == lhs.off - block.off)
                {
                    log_printf(
                        "%s%*s%s%s\n",
                        emph1_style(),
                        indent - max_indent,
                        "",
                        lhs_text,
                        reset_style());
                    goto lhs_text_done;
                }

                if (source[block.off + i++] == '\n')
                    break;
            }
    lhs_text_done:;
    }

    return gutter_indent;
}

void
log_excerpt_vimpl(
    int gutter_indent, const char* severity, const char* fmt, va_list ap)
{
    struct varef args;
    va_copy(args.ap, ap);

    log_printf("%*s = ", gutter_indent - 1, "");
    fprintf_with_color(severity);
    vfprintf_with_color(fmt, &args);
}

void
log_hex_ascii(const void* data, int len)
{
    int i;
    for (i = 0; i != 16; ++i)
        log_printf("%c  ", "0123456789ABCDEF"[i]);
    log_putc(' ');
    for (i = 0; i != 16; ++i)
        log_putc("0123456789ABCDEF"[i]);
    log_putc('\n');

    for (i = 0; i < len;)
    {
        int  j;
        uint8_t c = ((const uint8_t*)data)[i];
        for (j = 0; j != 16; ++j)
        {
            if (i + j < len)
                log_printf("%02x ", c);
            else
                log_printf("   ");
        }

        log_printf(" ");
        for (j = 0; j != 16 && i + j != len; ++j)
        {
            if (c >= 32 && c < 127) /* printable ascii */
                log_putc(c);
            else
                log_putc('.');
        }

        log_printf("\n");
        i += 16;
    }
}
