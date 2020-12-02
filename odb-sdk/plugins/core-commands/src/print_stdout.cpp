#include "core-commands/config.hpp"
#include <cstdio>

ODBPLUGIN_API const char* print_stdout_keyword = "print stdout";
ODBPLUGIN_API const char* print_stdout_typeinfo = "0(S)";
ODBPLUGIN_API const char* print_stdout_helpfile = "str.html";
ODBPLUGIN_API extern "C" void print_stdout(const char* str)
{
    puts(str);
}
