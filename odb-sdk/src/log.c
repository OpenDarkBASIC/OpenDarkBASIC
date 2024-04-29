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
struct varef
{
    va_list ap;
};
static const char*
process_standard_format(FILE* fp, const char* fmt, struct varef* args)
{
    int i = 0;
    char subfmt[16];
    subfmt[i++] = *fmt++;
    do {
        subfmt[i++] = *fmt++;
    } while (i != 15 && !is_ascii_alpha(fmt[-1]) && fmt[-1] != '%');

    subfmt[i] = '\0';
    vfprintf(fp, subfmt, args->ap);

    /* Have to advance to next argument */
    /* XXX: Does this work on all platforms? */
    //(void)va_arg(*ap, void*);

    return fmt;
}

/* ------------------------------------------------------------------------- */
static const char* debug_style(void) { return FG_YELLOW; }
static const char* info_style(void)  { return FGB_WHITE; }
static const char* note_style(void)  { return FGB_MAGENTA; }
static const char* warn_style(void)  { return FGB_YELLOW; }
static const char* err_style(void)   { return FGB_RED; }
static const char* emph_style(void)  { return FGB_WHITE; }
static const char*
next_control_sequence(const char* fmt, int* i)
{
    if (!fmt[*i])
        return NULL;

    switch (fmt[(*i)++])
    {
        case 'd': return debug_style();
        case 'i': return info_style();
        case 'n': return note_style();
        case 'w': return warn_style();
        case 'e':
            if (memcmp(&fmt[*i], "mph", 3) == 0)
            {
                (*i) += 3;
                return emph_style();
            }
            return err_style();
    }

    return NULL;
}
static const char*
process_color_format(FILE* fp, const char* fmt, struct varef* args)
{
    int i;
    const char* ctrl_seq;
    const char* content;
    for (i = 0; next_control_sequence(fmt + 1, &i); ) {}

    if (fmt[i] != ':')
    {
        putc(*fmt, fp);
        return fmt + 1;
    }

    for (i = 0; (ctrl_seq = next_control_sequence(fmt + 1, &i)) != NULL; )
        fprintf(fp, "%s", ctrl_seq);
    content = fmt + i + 1;

    while (*content)
    {
        if (*content == '{')
        {
            content = process_color_format(fp, content, args);
            for (i = 0; (ctrl_seq = next_control_sequence(fmt + 1, &i)) != NULL; )
                fprintf(fp, "%s", ctrl_seq);
        }
        else if (*content == '%')
            content = process_standard_format(fp, content, args);
        else if (*content == '}')
        {
            fprintf(fp, "\033[0m");
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
    struct varef args;
    va_copy(args.ap, ap);
    fprintf_with_color(stderr, group);
    fprintf_with_color(stderr, severity);
    vfprintf_with_color(stderr, fmt, &args);
}
