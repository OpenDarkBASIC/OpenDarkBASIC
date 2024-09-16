#include "core-commands/config.h"
#include <stdlib.h>

ODB_COMMAND1(
    /* clang-format off */
    int, val, const char* str,
    /* clang-format on */
    NAME("VAL"),
    BRIEF(""),
    DESCRIPTION(""),
    PARAMETER1("Text", ""),
    RETURNS(""),
    EXAMPLE(""),
    SEE_ALSO(""))
{
    return atoi(str);
}
