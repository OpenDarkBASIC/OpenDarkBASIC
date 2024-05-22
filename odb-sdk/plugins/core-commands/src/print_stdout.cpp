#include "core-commands/config.h"
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

// ODB_COMMAND3(
//     /* clang-format off */
//     void*, call_dll_vararg, int dll_number, const char* func_name, ...,
//     /* clang-format on */
//     NAME("CALL DLL"),
//     BRIEF("This command will call a function of a loaded DLL."),
//     DESCRIPTION(
//         "The DLL Number must be an integer value between 1 and 256. The DLL "
//         "Number points to the DLL previously loaded. The Function String is "
//         "the name of the function described in the export table of the DLL. "
//         "You can optionally have up to 9 parameters of integer, real or
//         string " "type providing the function you are calling matches the
//         parameters " "exactly. You can optionally return a value of integer,
//         real or string " "type providing the function exports the same
//         type."),
//     PARAMETER1(
//         "DLL Number",
//         "The DLL Number must be an integer value between 1 and 256"),
//     PARAMETER2(
//         "Function Name",
//         "The function string is the name of the function described in the "
//         "export table of the DLL"),
//     PARAMETER3(
//         "[Args]",
//         "You can optionally have up to 9 arguments to pass to the DLL "
//         "function"),
//     RETURNS("Returns an integer, float, or string"),
//     EXAMPLE(
//         "SM_CXSCREEN = 0\n"
//         "SM_CYSCREEN = 1\n"
//         "LOAD DLL \"User32.DLL\", 1\n"
//         "maxwidth = CALL DLL(1, \"GetSystemMetrics\", SM_CXSCREEN)\n",
//         "maxheight = CALL DLL(1, \"GetSystemMetrics\", SM_CYSCREEN)\n"
//         "CLOSE DLL 1\n"),
//     SEE_ALSO())
//{
//     return NULL;
// }

ODB_COMMAND1(
    /* clang-format off */
    void, print_str, const char* str,
    /* clang-format on */
    NAME("PRINT"),
    BRIEF("This command will print text, numbers, variables and strings to the "
          "screen."),
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
    SEE_ALSO("PRINTC", "STR$"))
{
    puts(str);
}
ODB_OVERLOAD1(
    /* clang-format off */
    void, print_i64, int64_t value,
    /* clang-format on */
    NAME("PRINT"))
{
    printf("%" PRIi64 "\n", value);
}
ODB_OVERLOAD1(
    /* clang-format off */
    void, print_u32, uint32_t value,
    /* clang-format on */
    NAME("PRINT"))
{
    printf("%" PRIu32 "\n", value);
}
ODB_OVERLOAD1(
    /* clang-format off */
    void, print_i32, int32_t value,
    /* clang-format on */
    NAME("PRINT"))
{
    printf("%" PRIi32 "\n", value);
}
ODB_OVERLOAD1(
    /* clang-format off */
    void, print_i16, uint16_t value,
    /* clang-format on */
    NAME("PRINT"))
{
    printf("%" PRIi16 "\n", value);
}
ODB_OVERLOAD1(
    /* clang-format off */
    void, print_i8, char value,
    /* clang-format on */
    NAME("PRINT"))
{
    printf("%c\n", value);
}
ODB_OVERLOAD1(
    /* clang-format off */
    void, print_f32, float value,
    /* clang-format on */
    NAME("PRINT"))
{
    printf("%f\n", value);
}
ODB_OVERLOAD1(
    /* clang-format off */
    void, print_f64, double value,
    /* clang-format on */
    NAME("PRINT"))
{
    printf("%f\n", value);
}
