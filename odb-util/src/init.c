#include "odb-util/backtrace.h"
#include "odb-util/init.h"
#include "odb-util/log.h"
#include "odb-util/mem.h"

/* ------------------------------------------------------------------------- */
int
odbutil_init(void)
{
    log_init();

    if (backtrace_init() < 0)
        goto backtrace_init_failed;
    if (mem_init() != 0)
        goto init_mem_failed;

    return 0;

init_mem_failed:
    backtrace_deinit();
backtrace_init_failed:
    return -1;
}

/* ------------------------------------------------------------------------- */
void
odbutil_deinit(void)
{
    mem_deinit();
    backtrace_deinit();
}

