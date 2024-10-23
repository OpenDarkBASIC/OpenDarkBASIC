#pragma once

#include "odb-compiler/config.h"
#include "odb-util/ospath.h"

struct ast;
struct cmd_list;

ODBCOMPILER_PUBLIC_API int
ast_export_print_fp(
    const struct ast*      ast,
    int                    root,
    FILE*                  fp,
    const char*            source,
    const struct cmd_list* cmds);
ODBCOMPILER_PUBLIC_API int
ast_export_dot(
    const struct ast*      ast,
    int                    root,
    struct ospathc         filepath,
    const char*            source,
    const struct cmd_list* cmds);

ODBCOMPILER_PUBLIC_API int
ast_export_dot_fp(
    const struct ast*      ast,
    int                    root,
    FILE*                  fp,
    const char*            source,
    const struct cmd_list* cmds);
