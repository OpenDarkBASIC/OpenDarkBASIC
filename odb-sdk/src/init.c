#include "odb-sdk/backtrace.h"
#include "odb-sdk/init.h"
#include "odb-sdk/log.h"
#include "odb-sdk/mem.h"

/* ------------------------------------------------------------------------- */
int
odbsdk_init(void)
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
odbsdk_deinit(void)
{
    mem_deinit();
    backtrace_deinit();
}

