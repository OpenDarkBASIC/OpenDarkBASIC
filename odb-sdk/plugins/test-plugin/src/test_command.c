#include "test-plugin/config.h"
#include <stdio.h>

ODB_COMMAND0("test command", "help/print.html", void, test_command0)
{
    puts("Hello from test_command!");
}

ODB_COMMAND1("test command", "help/print.html", void, test_command1, int value)
{
    printf("Hello! Value is %d\n", value);
}
