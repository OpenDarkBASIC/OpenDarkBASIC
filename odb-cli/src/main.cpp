#include "odb-cli/Actions.argdef.hpp"

extern "C" {
#include "odb-sdk/init.h"
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

    success = parseCommandLine(argc, argv);

odbsdk_init_failed:
    odbsdk_threadlocal_deinit();
odbsdk_tl_init_failed:
    return success ? 0 : -1;
}
