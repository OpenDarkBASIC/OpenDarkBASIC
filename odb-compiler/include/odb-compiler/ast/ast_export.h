#pragma once

#include "odb-compiler/config.h"
#include "odb-sdk/utf8.h"

struct ast;
struct db_source;
struct cmd_list;

ODBCOMPILER_PUBLIC_API int
ast_export_dot(
    const struct ast*       ast,
    struct utf8_view        filepath,
    const struct db_source* source,
    const struct cmd_list*  commands);

ODBCOMPILER_PUBLIC_API int
ast_export_dot_fp(
    const struct ast*       ast,
    FILE*                   fp,
    const struct db_source* source,
    const struct cmd_list*  commands);
