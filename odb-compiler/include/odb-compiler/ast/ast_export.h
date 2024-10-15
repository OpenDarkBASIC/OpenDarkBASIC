#pragma once

#include "odb-compiler/config.h"
#include "odb-util/ospath.h"

struct ast;
struct cmd_list;

ODBCOMPILER_PUBLIC_API int
ast_export_dot(
    const struct ast*      ast,
    struct ospathc         filepath,
    const char*            source_text,
    const struct cmd_list* commands);

ODBCOMPILER_PUBLIC_API int
ast_export_dot_fp(
    const struct ast*      ast,
    FILE*                  fp,
    const char*            source_text,
    const struct cmd_list* commands);
