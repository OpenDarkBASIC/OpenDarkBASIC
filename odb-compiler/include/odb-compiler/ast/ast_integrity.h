#pragma once

#include "odb-compiler/config.h"
#include "odb-compiler/parser/db_source.h"

struct ast;
struct cmd_list;

ODBCOMPILER_PUBLIC_API int
ast_verify_connectivity(
    const struct ast*      ast,
    struct db_source       source,
    const struct cmd_list* cmds);
