#include "odb-compiler/sdk/sdk_type.h"

const char*
sdk_type_to_name(enum sdk_type sdk_type)
{
    switch (sdk_type)
    {
        case SDK_ODB: return "ODB";
        case SDK_DBPRO: return "DBPro";
    }

    return "";
}
