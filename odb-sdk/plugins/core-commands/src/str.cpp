#include "core-commands/config.hpp"
#include "odb-sdk/Str.hpp"

ODBPLUGIN_API const char* str_name = "str$";
ODBPLUGIN_API const char* str_typeinfo = "S(L)";
ODBPLUGIN_API const char* str_helpfile = "str.html";
ODBPLUGIN_API extern "C" char* str(int value)
{
    return nullptr;
}
