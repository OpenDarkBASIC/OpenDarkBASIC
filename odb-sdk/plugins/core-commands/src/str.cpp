#include "core-commands/config.hpp"

ODBPLUGIN_API const char* str_name = "str$";
ODBPLUGIN_API const char* str_typeinfo = "S(L)";
ODBPLUGIN_API const char* str_helpfile = "str.html";
extern "C" ODBPLUGIN_API char* str(int value)
{
    return nullptr;
}
