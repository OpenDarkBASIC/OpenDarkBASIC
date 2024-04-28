#include "odb-sdk/log.h"
#include "odb-sdk/cli_colors.h"
#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
static int
is_ascii_alpha(char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z');
}
static int
is_ascii_numeric(char c)
{
    return c >= '0' && c <= '9';
}

/* ------------------------------------------------------------------------- */
static const char*
process_standard_format(FILE* fp, const char* fmt, va_list ap)
{
    int i = 0;
    char subfmt[16];
    subfmt[i++] = *fmt++;
    do {
        subfmt[i++] = *fmt++;
    } while (i != 15 && !is_ascii_alpha(fmt[-1]) && fmt[-1] != '%');

    subfmt[i] = '\0';
    vfprintf(fp, subfmt, ap);

    return fmt;
}

/* ------------------------------------------------------------------------- */
static const char*
process_color_format(FILE* fp, const char* fmt, va_list ap)
{
    char fg[9] = "\033[\0     ";
    char bg[9] = "\033[\0     ";
    char set_fg = 0;
    char set_bg = 0;
    fmt += 2;  /* "%{" */

    /* Set foreground color */
    if (*fmt == 'b')  /* bold */
    {
        strcat(fg, "1;");
        fmt++;
    }
    else
        strcat(fg, "22;");
    if (is_ascii_numeric(*fmt))
    {
        strcat(fg, "3");
        strncat(fg, fmt++, 1);
        strcat(fg, "m");
        set_fg = 1;
    }

    /* Set background color */
    if (*fmt == 'b')  /* bold */
    {
        strcat(bg, "1;");
        fmt++;
    }
    else
        strcat(bg, "22;");
    if (is_ascii_numeric(*fmt))
    {
        strcat(bg, "4");
        strncat(bg, fmt++, 1);
        strcat(bg, "m");
        set_bg = 1;
    }

    if (*fmt == ':')
        fmt++;

    if (set_fg)
        fprintf(fp, "%s", fg);
    if (set_bg)
        fprintf(fp, "%s", bg);

    while (*fmt)
    {
        if (fmt[0] == '%' && fmt[1] == '{')
        {
            fmt = process_color_format(fp, fmt, ap);
            if (set_fg)
                fprintf(fp, "%s", fg);
            if (set_bg)
                fprintf(fp, "%s", bg);
        }
        else if (fmt[0] == '%')
            fmt = process_standard_format(fp, fmt, ap);
        else if (fmt[0] == '}')
        {
            fmt++;
            break;
        }
        else
            putc(*fmt++, fp);
    }
    
    fprintf(fp, "\033[0m");
    
    return fmt;
}

/* ------------------------------------------------------------------------- */
static void
vfprintf_with_color(FILE* fp, const char* fmt, va_list ap)
{
    while (*fmt)
    {
        if (fmt[0] == '%' && fmt[1] == '{')
            fmt = process_color_format(fp, fmt, ap);
        else if (fmt[0] == '%')
            fmt = process_standard_format(fp, fmt, ap);
        else
            putc(*fmt++, fp);
    }
}

static void
fprintf_with_color(FILE* fp, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf_with_color(fp, fmt, ap);
    va_end(ap);
}

/* ------------------------------------------------------------------------- */
void
log_raw(const char* severity, const char* group, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_vraw(severity, group, fmt, ap);
    va_end(ap);
}

/* ------------------------------------------------------------------------- */
void
log_vraw(const char* severity, const char* group, const char* fmt, va_list ap)
{
    fprintf_with_color(stderr, severity);
    fprintf_with_color(stderr, group);
    vfprintf_with_color(stderr, fmt, ap);
}

