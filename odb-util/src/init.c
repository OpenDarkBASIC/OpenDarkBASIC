#include "odb-util/backtrace.h"
#include "odb-util/init.h"
#include "odb-util/log.h"
#include "odb-util/mem.h"

/* ------------------------------------------------------------------------- */
int
odbutil_init(void)
{
    if (backtrace_init() != 0)
        goto backtrace_init_failed;
    if (mem_init() != 0)
        goto init_mem_failed;
    if (log_init() != 0)
        goto log_init_failed;

    return 0;

log_init_failed:
    mem_deinit();
init_mem_failed:
    backtrace_deinit();
backtrace_init_failed:
    return -1;
}

/* ------------------------------------------------------------------------- */
int
odbutil_deinit(void)
{
    int leaks;
    log_deinit();
    leaks = mem_deinit();
    backtrace_deinit();
    return leaks;
}
