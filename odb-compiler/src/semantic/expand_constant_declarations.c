#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"

static int
expand_constant_declarations(
    struct ast*            ast,
    const struct cmd_list* cmds,
    const char*            source_filename,
    struct db_source       source)
{
    return 0;
}

static const struct semantic_check* depends[] = {NULL};
const struct semantic_check         semantic_expand_constant_declarations
    = {depends, expand_constant_declarations};
