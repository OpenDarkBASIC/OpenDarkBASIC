#include "core-commands/config.h"
#include <cstdio>

ODB_COMMAND1("stdout", "help/stdout.html", void, print_stdout_str, const char* str)
{
    puts(str);
}
ODB_COMMAND1("stdout", "help/stdout.html", void, print_stdout_int, int value)
{
    printf("%d\n", value);
}

