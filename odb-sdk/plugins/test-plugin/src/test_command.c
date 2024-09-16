#include "test-plugin/config.h"
#include <stdio.h>

ODB_COMMAND3(
    /* clang-format off */
    void, test_command, const char* str, char b2, char byte,
    /* clang-format on */
    NAME("TEST COMMAND"),
    BRIEF(),
    DESCRIPTION(),
    PARAMETER1("a", ""),
    PARAMETER2("b", ""),
    PARAMETER3("c", ""),
    RETURNS(),
    EXAMPLE(),
    SEE_ALSO())
{
    puts("Hello!");
}
