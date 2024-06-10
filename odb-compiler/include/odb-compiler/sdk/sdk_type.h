#pragma once

#include "odb-compiler/config.h"

/* clang-format off */
#define SDK_TYPE_LIST \
    X(ODB)            \
    X(DBPRO)
/* clang-format on */

enum sdk_type
{
#define X(name) SDK_##name,
    SDK_TYPE_LIST
#undef X
};

ODBCOMPILER_PUBLIC_API const char*
sdk_type_to_name(enum sdk_type sdk_type);
