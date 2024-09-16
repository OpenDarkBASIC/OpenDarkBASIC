#include "odb-util/backtrace.h"
#include <execinfo.h>
#include <stdlib.h>

/* ------------------------------------------------------------------------- */
int
backtrace_init(void)
{
    return 0;
}

/* ------------------------------------------------------------------------- */
void
backtrace_deinit(void)
{
}

/* ------------------------------------------------------------------------- */
char**
backtrace_get(int* size)
{
    void* array[ODBUTIL_MEM_BACKTRACE_SIZE];
    char** strings;

    *size = backtrace(array, ODBUTIL_MEM_BACKTRACE_SIZE);
    strings = backtrace_symbols(array, *size);

    return strings;
}

/* ------------------------------------------------------------------------- */
void
backtrace_free(char** bt)
{
    free(bt);
}
