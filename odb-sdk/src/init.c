#include "odb-sdk/backtrace.h"
#include "odb-sdk/fs.h"
#include "odb-sdk/mem.h"
#include "odb-sdk/init.h"

/* ------------------------------------------------------------------------- */
int
odbsdk_init(void)
{
    if (backtrace_init() < 0)
        goto backtrace_init_failed;
    if (fs_init() < 0)
        goto fs_init_failed;

    return 0;

    fs_init_failed          : backtrace_deinit();
    backtrace_init_failed   : return -1;
}

/* ------------------------------------------------------------------------- */
void
odbsdk_deinit(void)
{
    fs_deinit();
    backtrace_deinit();
}

/* ------------------------------------------------------------------------- */
int
odbsdk_threadlocal_init(void)
{
    return mem_threadlocal_init();
}

/* ------------------------------------------------------------------------- */
void
odbsdk_threadlocal_deinit(void)
{
    mem_threadlocal_deinit();
}
