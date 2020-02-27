#include "odbc/cli/Args.hpp"
#include "odbc/logging/Log.hpp"
#include <cstdio>

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    odbc::log::init();

    Args args;
    if (!args.parse(argc, argv))
        return 1;

    return 0;
}
