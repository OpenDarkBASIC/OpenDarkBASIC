#pragma once

#include "odb-compiler/config.h"
#include "odb-compiler/parser/db_source.h"
#include "odb-util/ospath.h"

struct ast;
struct cmd_list;

/* Writes to <cwd>/(basename of <filename>).ast */
ODBCOMPILER_PUBLIC_API int
ast_export_filename(
    const struct ast*      ast,
    const char*            filename,
    struct db_source       source,
    const struct cmd_list* cmds);

ODBCOMPILER_PUBLIC_API int
ast_export(
    const struct ast*      ast,
    struct ospathc         filepath,
    struct db_source       source,
    const struct cmd_list* cmds);

ODBCOMPILER_PUBLIC_API int
ast_export_fp(
    const struct ast*      ast,
    FILE*                  fp,
    struct db_source       source,
    const struct cmd_list* cmds);
