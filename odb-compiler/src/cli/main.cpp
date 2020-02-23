#include "odbc/cli/Args.hpp"
#include <cstdio>

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
    Args args;
    if (!args.parse(argc, argv))
        return 1;

    return 0;
}
