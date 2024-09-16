#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "odb-runtime/config.h"
#include "odb-util/init.h"

static void
exit_process(void)
{
#if defined(_WIN32)
    ExitProcess(0);
#else
    _exit(0);
#endif
}

ODBRUNTIME_API int
odbrt_init(void)
{
    return odbutil_init();
}

ODBRUNTIME_API void
odbrt_exit(void)
{
    odbutil_deinit();
    exit_process();
}
