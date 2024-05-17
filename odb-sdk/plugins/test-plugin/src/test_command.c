#include "test-plugin/config.h"
#include <stdio.h>

ODB_COMMAND0("test command", "help/print.html", void, test_command0)
{
    puts("Hello from test_command!");
}

ODB_COMMAND1(
    "test command", "help/print.html", void, test_command1, const char* str)
{
    puts(str);
}

ODB_COMMAND2(
    "test command",
    "help/print.html",
    void,
    test_command2,
    const char* str,
    int         value)
{
    printf("%s: %d\n", str, value);
}
