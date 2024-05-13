#include "core-commands/config.hpp"
#include <cstdio>

extern "C" {
ODBPLUGIN_API const char* print_stdout_name = "stdout";
ODBPLUGIN_API const char* print_stdout_typeinfo = "0(S)";
ODBPLUGIN_API const char* print_stdout_helpfile = "str.html";
ODBPLUGIN_API void print_stdout(const char* str)
{
    puts(str);
}
}

