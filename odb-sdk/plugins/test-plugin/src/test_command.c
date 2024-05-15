#include "test-plugin/config.h"
#include <stdio.h>

__attribute__((visibility("default"))) const char* test_command_name = "test command";
int foo(int a, int b) {
    return a+b;
}
//ODB_COMMAND0(
//    "test command",
//    "help/print.html",
//    void, test_command)
//{
//    puts("Hello from test_command!");
//}
