#include "odb-compiler/cli/Args.hpp"
#include "odb-sdk/Plugin.hpp"
#include "odb-sdk/Log.hpp"
#include <cstdio>

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    odb::log::init();

    Args args;
    if (!args.parse(argc, argv))
        return 1;

    return 0;
}
