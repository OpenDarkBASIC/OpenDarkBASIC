#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "odb-runtime/config.h"
#include "odb-util/init.h"

static void
exit_process(int exit_code)
{
#if defined(_WIN32)
    ExitProcess(exit_code);
#else
    _exit(exit_code);
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
    exit_process(odbutil_deinit());
}
