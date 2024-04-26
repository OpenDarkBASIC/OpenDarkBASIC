#include "odb-sdk/log.h"
#include "odb-sdk/cli_colors.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

static ODBSDK_THREADLOCAL const char* g_prefix = "";
static ODBSDK_THREADLOCAL const char* g_col_set = "";
static ODBSDK_THREADLOCAL const char* g_col_clr = "";

static FILE* g_log = NULL;

/* ------------------------------------------------------------------------- */
void
log_set_prefix(const char* prefix)
{
    g_prefix = prefix;
}
void
log_set_colors(const char* set, const char* clear)
{
    g_col_set = set;
    g_col_clr = clear;
}

/* ------------------------------------------------------------------------- */
void
log_file_open(const char* log_file)
{
    if (g_log)
    {
        log_warn("log_file_open() called, but a log file is already open. Closing previous file...\n");
        log_file_close();
    }

    log_info("Opening log file \"%s\"\n", log_file);
    g_log = fopen(log_file, "w");
    if (g_log == NULL)
        log_err("Failed to open log file \"%s\": %s\n", log_file, strerror(errno));
}
void
log_file_close()
{
    if (g_log)
    {
        log_info("Closing log file\n");
        fclose(g_log);
        g_log = NULL;
    }
}

/* ------------------------------------------------------------------------- */
void
log_raw(const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);

    if (g_log)
    {
        va_start(va, fmt);
        vfprintf(g_log, fmt, va);
        va_end(va);
        fflush(g_log);
    }
}

/* ------------------------------------------------------------------------- */
void
log_dbg(const char* fmt, ...)
{
    va_list va;
    fprintf(stderr, "[" COL_N_YELLOW "Debug" COL_RESET "] %s%s%s", g_col_set, g_prefix, g_col_clr);
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);

    if (g_log)
    {
        fprintf(g_log, "[Debug] %s", g_prefix);
        va_start(va, fmt);
        vfprintf(g_log, fmt, va);
        va_end(va);
        fflush(g_log);
    }
}

/* ------------------------------------------------------------------------- */
void
log_info(const char* fmt, ...)
{
    va_list va;
    fprintf(stderr, "[" COL_B_WHITE "Info " COL_RESET "] %s%s%s", g_col_set, g_prefix, g_col_clr);
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);

    if (g_log)
    {
        fprintf(g_log, "[Info ] %s", g_prefix);
        va_start(va, fmt);
        vfprintf(g_log, fmt, va);
        va_end(va);
        fflush(g_log);
    }
}

/* ------------------------------------------------------------------------- */
void
log_warn(const char* fmt, ...)
{
    va_list va;
    fprintf(stderr, "[" COL_B_YELLOW "Warn " COL_RESET "] %s%s%s", g_col_set, g_prefix, g_col_clr);
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);

    if (g_log)
    {
        fprintf(g_log, "[Warn ] %s", g_prefix);
        va_start(va, fmt);
        vfprintf(g_log, fmt, va);
        va_end(va);
        fflush(g_log);
    }
}

/* ------------------------------------------------------------------------- */
void
log_err(const char* fmt, ...)
{
    va_list va;
    fprintf(stderr, "[" COL_B_RED "Error" COL_RESET "] %s%s%s", g_col_set, g_prefix, g_col_clr);
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);

    if (g_log)
    {
        fprintf(g_log, "[Error] %s", g_prefix);
        va_start(va, fmt);
        vfprintf(g_log, fmt, va);
        va_end(va);
        fflush(g_log);
    }
}

/* ------------------------------------------------------------------------- */
void
log_note(const char* fmt, ...)
{
    va_list va;
    fprintf(stderr, "[" COL_B_MAGENTA "NOTE " COL_RESET "] %s%s%s" COL_B_YELLOW, g_col_set, g_prefix, g_col_clr);
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
    fprintf(stderr, COL_RESET);

    if (g_log)
    {
        fprintf(g_log, "[NOTE ] %s", g_prefix);
        va_start(va, fmt);
        vfprintf(g_log, fmt, va);
        va_end(va);
        fflush(g_log);
    }
}

/* ------------------------------------------------------------------------- */
void
log_mem_warn(const char* fmt, ...)
{
    va_list va;
    fprintf(stderr, "[" COL_B_YELLOW "Warn " COL_RESET "] " COL_B_BLUE "mem: " COL_RESET);
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);

    if (g_log)
    {
        fprintf(g_log, "[Warn ] mem: ");
        va_start(va, fmt);
        vfprintf(g_log, fmt, va);
        va_end(va);
        fflush(g_log);
    }
}
void
log_mem_err(const char* fmt, ...)
{
    va_list va;
    fprintf(stderr, "[" COL_B_RED "Error" COL_RESET "] " COL_B_BLUE "mem: " COL_RESET);
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);

    if (g_log)
    {
        fprintf(g_log, "[Error] mem: ");
        va_start(va, fmt);
        vfprintf(g_log, fmt, va);
        va_end(va);
        fflush(g_log);
    }
}
void
log_mem_note(const char* fmt, ...)
{
    va_list va;
    fprintf(stderr, "[" COL_B_MAGENTA "NOTE " COL_RESET "] " COL_B_BLUE "mem: " COL_RESET);
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);

    if (g_log)
    {
        fprintf(g_log, "[NOTE ] mem: ");
        va_start(va, fmt);
        vfprintf(g_log, fmt, va);
        va_end(va);
        fflush(g_log);
    }
}
