#include "odb-cli/AST.hpp"
#include "odb-cli/Actions.argdef.hpp"
#include "odb-cli/Commands.hpp"
#include "odb-cli/SDK.hpp"

extern "C" {
#include "odb-sdk/init.h"
}

// ----------------------------------------------------------------------------
int
main(int argc, char** argv)
{
    bool success = false;

    if (odbsdk_init() != 0)
        goto odbsdk_init_failed;

    initSDK();
    initCommands();
    initAST();

    success = parseCommandLine(argc, argv);

    deinitAST();
    deinitCommands();
    deinitSDK();

    odbsdk_deinit();

odbsdk_init_failed:
    return success ? 0 : -1;
}
