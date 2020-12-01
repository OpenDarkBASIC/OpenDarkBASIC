#include <stdio.h>

#define ODB_API

ODB_API const char* odbPrint1_keyword = "print";
ODB_API const char* odbPrint1_typeinfo = "v(s)";
ODB_API const char* odbPrint1_helpfile = "help/print.html";
ODB_API void odbPrint1(const char* msg)
{
    puts(msg);
}

ODB_API const char* odbPrint2_keyword = "print";
ODB_API const char* odbPrint2_typeinfo = "v(iis)";
ODB_API const char* odbPrint2_helpfile = "help/print.html";
ODB_API void odbPrint2(int x, int y, const char* msg)
{
    printf("%d, %d: %s\n", x, y, msg);
}
