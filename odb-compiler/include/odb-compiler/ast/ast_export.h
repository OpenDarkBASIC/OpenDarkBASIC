#pragma once

#include "odb-compiler/config.h"
#include "odb-util/ospath.h"

struct ast;
struct cmd_list;

/* Writes to <cwd>/(basename of <filename>).ast */
ODBCOMPILER_PUBLIC_API int
ast_export_basename(
    const struct ast*      ast,
    struct ospathc         filename,
    struct utf8_view       source,
    const struct cmd_list* cmds);

ODBCOMPILER_PUBLIC_API int
ast_export(
    const struct ast*      ast,
    struct ospathc         filepath,
    struct utf8_view       source,
    const struct cmd_list* cmds);

ODBCOMPILER_PUBLIC_API int
ast_export_fp(
    const struct ast*      ast,
    FILE*                  fp,
    struct utf8_view       source,
    const struct cmd_list* cmds);
