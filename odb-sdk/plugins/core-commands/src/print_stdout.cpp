#include "core-commands/config.h"
#include <cstdio>

ODB_COMMAND3(
    void*, call_dll_vararg, int dll_number, const char* func_name, ...,
    NAME("CALL DLL"),
    DESCRIPTION("This command will call a function of a loaded DLL."),
    PARAMETER1(
        "DLL Number",
        "The DLL Number must be an integer value between 1 and 256"),
    PARAMETER2(
        "Function Name",
        "The function string is the name of the function described in the "
        "export table of the DLL"),
    PARAMETER3(
        "[Params]",
        "You can optionally have up to 9 parameters to pass to the DLL "
        "function"),
    RETURNS(
        "Returns an integer, float, or string"),
    EXAMPLE(
        "SM_CXSCREEN = 0\n"
        "SM_CYSCREEN = 1\n"
        "LOAD DLL \"User32.DLL\", 1\n"
        "maxwidth = CALL DLL(1, \"GetSystemMetrics\", SM_CXSCREEN)\n",
        "maxheight = CALL DLL(1, \"GetSystemMetrics\", SM_CYSCREEN)\n"
        "CLOSE DLL 1\n"),
    SEE_ALSO(""))
{
    return NULL;
}

//ODB_COMMAND1(
//    "stdout", "help/stdout.html", void, print_stdout_str, const char* str)
//{
//    puts(str);
//}
//ODB_COMMAND1("stdout", "help/stdout.html", void, print_stdout_int, int value)
//{
//    printf("%d\n", value);
//}
