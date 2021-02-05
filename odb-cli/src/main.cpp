#include "odb-cli/Actions.hpp"
#include "odb-sdk/Log.hpp"

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    if (parseCommandLine(argc, argv) == false)
        return -1;

    return 0;
}
