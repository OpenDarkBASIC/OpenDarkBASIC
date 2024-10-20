#include "core-commands/config.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

ODB_COMMAND1(
    /* clang-format off */
    void, print_stdout_str, const char* str,
    /* clang-format on */
    NAME("PRINT STDOUT"),
    BRIEF("This command will print text, numbers, variables and strings to "
          "standard output (stdout)"),
    DESCRIPTION(
        "You can position where the text will print using the SET CURSOR "
        "command. You can separate items you wish to print on the same line by "
        "using either a semi-colon or a comma. If you add a semi-colon at the "
        "end of your print list, the next PRINT command will add to the end of "
        "the last print line. This command can only produce text on a single "
        "line, so you should refrain from using carriage return characters in "
        "the text string. Any carriage return symbols will be ignored."),
    PARAMETER1(
        "Print Statements",
        "Text, integer, float, or any other primitive type to print. Use the "
        "STR$() command to convert other types to strings for printing."),
    RETURNS(),
    EXAMPLE("PRINT \"Hello, World!\"\n"
            "PRINT 123\n"
            "PRINT 420.69\n"
            "WAIT KEY\n"),
    SEE_ALSO("PRINT", "PRINTC", "STR$"))
{
    puts(str);
    fflush(stdout);
}
ODB_OVERLOAD1(
    /* clang-format off */
    void, print_stdout_i64, int64_t value,
    /* clang-format on */
    NAME("PRINT STDOUT"))
{
    printf("%" PRIi64 "\n", value);
    fflush(stdout);
}
ODB_OVERLOAD1(
    /* clang-format off */
    void, print_stdout_f64, double value,
    /* clang-format on */
    NAME("PRINT STDOUT"))
{
    printf("%f\n", value);
    fflush(stdout);
}

ODB_COMMAND1(
    /* clang-format off */
    void, printc_stdout_str, const char* str,
    /* clang-format on */
    NAME("PRINTC STDOUT"),
    BRIEF("This command will print text, numbers, variables and strings to "
          "standard output (stdout)"),
    DESCRIPTION(
        "You can position where the text will print using the SET CURSOR "
        "command. You can separate items you wish to print on the same line by "
        "using either a semi-colon or a comma. If you add a semi-colon at the "
        "end of your print list, the next PRINT command will add to the end of "
        "the last print line. This command can only produce text on a single "
        "line, so you should refrain from using carriage return characters in "
        "the text string. Any carriage return symbols will be ignored."),
    PARAMETER1(
        "Print Statements",
        "Text, integer, float, or any other primitive type to print. Use the "
        "STR$() command to convert other types to strings for printing."),
    RETURNS(),
    EXAMPLE("PRINT \"Hello, World!\"\n"
            "PRINT 123\n"
            "PRINT 420.69\n"
            "WAIT KEY\n"),
    SEE_ALSO("PRINT", "PRINTC", "STR$"))
{
    printf("%s", str);
    fflush(stdout);
}
ODB_OVERLOAD1(
    /* clang-format off */
    void, printc_stdout_i64, int64_t value,
    /* clang-format on */
    NAME("PRINTC STDOUT"))
{
    printf("%" PRIi64, value);
    fflush(stdout);
}
ODB_OVERLOAD1(
    /* clang-format off */
    void, printc_stdout_f64, double value,
    /* clang-format on */
    NAME("PRINTC STDOUT"))
{
    printf("%f", value);
    fflush(stdout);
}

ODB_COMMAND1(
    /* clang-format off */
    void, print_str, const char* str,
    /* clang-format on */
    NAME("PRINT"),
    BRIEF("This command will print text, numbers, variables and strings to "
          "the screen."),
    DESCRIPTION(
        "You can position where the text will print using the SET CURSOR "
        "command. You can separate items you wish to print on the same line by "
        "using either a semi-colon or a comma. If you add a semi-colon at the "
        "end of your print list, the next PRINT command will add to the end of "
        "the last print line. This command can only produce text on a single "
        "line, so you should refrain from using carriage return characters in "
        "the text string. Any carriage return symbols will be ignored."),
    PARAMETER1(
        "Print Statements",
        "Text, integer, float, or any other primitive type to print. Use the "
        "STR$() command to convert other types to strings for printing."),
    RETURNS(),
    EXAMPLE("PRINT \"Hello, World!\"\n"
            "PRINT 123\n"
            "PRINT 420.69\n"
            "WAIT KEY\n"),
    SEE_ALSO("PRINT", "PRINTC", "STR$"))
{
    puts(str);
    fflush(stdout);
}
ODB_OVERLOAD1(
    /* clang-format off */
    void, print_i64, int64_t value,
    /* clang-format on */
    NAME("PRINT"))
{
    printf("%" PRIi64 "\n", value);
    fflush(stdout);
}
ODB_OVERLOAD1(
    /* clang-format off */
    void, print_f64, double value,
    /* clang-format on */
    NAME("PRINT"))
{
    printf("%f\n", value);
    fflush(stdout);
}
