#include "odb-cli/Actions.argdef.hpp"
#include "odb-sdk/Log.hpp"

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    if (!parseCommandLine(argc, argv))
        return -1;

    return 0;
}
