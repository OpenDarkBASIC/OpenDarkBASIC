#include "test-plugin/config.hpp"
#include <stdio.h>

ODBPLUGIN_API const char* test_command_keyword = "test command";
ODBPLUGIN_API const char* test_command_typeinfo = "0()";
ODBPLUGIN_API const char* test_command_helpfile = "help/print.html";
ODBPLUGIN_API void test_command(void)
{
    puts("Hello from test_command!");
}

