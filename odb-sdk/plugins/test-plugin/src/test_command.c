#include "test-plugin/config.h"
#include <stdio.h>

ODB_COMMAND0(
    "test command",
    "help/print.html",
    void, test_command)
{
    puts("Hello from test_command!");
}
