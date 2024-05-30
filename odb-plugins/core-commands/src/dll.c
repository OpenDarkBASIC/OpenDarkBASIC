#include "core-commands/config.h"

ODB_COMMAND2(
    /* clang-format off */
    void, load_dll, const char* dll_name, int dll_number,
    /* clang-format on */
    NAME("LOAD DLL"),
    BRIEF(""),
    DESCRIPTION(""),
    PARAMETER1("File Name", ""),
    PARAMETER2("DLL Number", ""),
    RETURNS(""),
    EXAMPLE(""),
    SEE_ALSO())
{
}

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

