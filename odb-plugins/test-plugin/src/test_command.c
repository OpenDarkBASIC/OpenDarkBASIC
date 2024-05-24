#include "test-plugin/config.h"
#include <stdio.h>

ODB_COMMAND1(
    /* clang-format off */
    void, test_command, const char* str,
    /* clang-format on */
    NAME("TEST COMMAND"),
    BRIEF(),
    DESCRIPTION(),
    PARAMETER1("", ""),
    RETURNS(),
    EXAMPLE(),
    SEE_ALSO())
{
    puts("Hello!");
}
