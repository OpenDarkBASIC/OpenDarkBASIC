#include "odb-cli/AST.hpp"
#include "odb-cli/Actions.argdef.hpp"
#include "odb-cli/Commands.hpp"
#include "odb-cli/SDK.hpp"

extern "C" {
#include "odb-sdk/init.h"
#include "odb-sdk/log.h"
}

/* ------------------------------------------------------------------------- */
#if defined(ODBSDK_PLATFORM_WINDOWS)
#else
#include <unistd.h>
#endif

static char
stream_is_terminal(FILE* fp)
{
#if defined(ODBSDK_PLATFORM_WINDOWS)
    return 1;
#else
    return isatty(fileno(fp));
#endif
}

// ----------------------------------------------------------------------------
int
main(int argc, char** argv)
{
    bool success = false;

    if (odbsdk_threadlocal_init() != 0)
        goto odbsdk_tl_init_failed;
    if (odbsdk_init() != 0)
        goto odbsdk_init_failed;

    log_configure(
        {[](const char* fmt, va_list ap) { vfprintf(stderr, fmt, ap); },
         stream_is_terminal(stderr)});

    initSDK();
    initCommands();
    initAST();

    success = parseCommandLine(argc, argv);

    deinitAST();
    deinitCommands();
    deinitSDK();

odbsdk_init_failed:
    odbsdk_threadlocal_deinit();
odbsdk_tl_init_failed:
    return success ? 0 : -1;
}
