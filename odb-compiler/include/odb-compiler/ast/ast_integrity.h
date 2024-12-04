#pragma once

#include "odb-compiler/config.h"

struct ast;
struct cmd_list;

ODBCOMPILER_PUBLIC_API int
ast_sanity_check(
    const struct ast* ast, const char* source, const struct cmd_list* cmds);
