#pragma once

#include "odb-compiler/config.h"
#include "odb-compiler/parser/db_source.h"

struct cmd_list;
struct plugin_list;

ODBCOMPILER_PUBLIC_API int
cmd_list_from_source(
    struct plugin_list** plugins,
    struct cmd_list*     cmds,
    const char*          filename,
    struct db_source     source);
