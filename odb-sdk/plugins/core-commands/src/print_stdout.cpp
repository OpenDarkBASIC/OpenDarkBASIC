#include "core-commands/config.h"
#include <cstdio>

ODB_COMMAND1("stdout", "help/stdout.html", void, print_stdout, const char* str)
{
    puts(str);
}

