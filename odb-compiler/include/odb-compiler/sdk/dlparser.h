#pragma once

#include "odb-compiler/config.h"
#include "odb-sdk/ospath.h"

ODBCOMPILER_PUBLIC_API int
dlparser_symbols_in_section(
    struct ospathc filepath,
    int            (*on_symbol)(const char* sym, void* user),
    void*          user);

ODBCOMPILER_PUBLIC_API int
dlparser_strings(
    struct ospathc filepath,
    int            (*on_string)(const char* sym, void* user),
    void*          user);
