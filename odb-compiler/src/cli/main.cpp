#include "odb-compiler/cli/Args.hpp"
#include "odb-compiler/commands/Command.hpp"
#include "odb-sdk/Log.hpp"

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    odb::log::init();

    Args args;
    if (!args.parse(argc, argv))
        return 1;

    return 0;
}
