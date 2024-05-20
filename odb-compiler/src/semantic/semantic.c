#include "odb-compiler/semantic/semantic.h"

int
semantic_checks_run(
    struct ast*            ast,
    const struct cmd_list* cmds,
    const char*            source_filename,
    struct db_source       source)
{
    if (semantic_expand_constant_declarations.execute(ast, cmds, source_filename, source) != 0) return -1;
    if (semantic_type_check_expressions.execute(ast, cmds, source_filename, source) != 0) return -1;
    if (semantic_resolve_cmd_overloads.execute(ast, cmds, source_filename, source) != 0) return -1;

    return 0;
}

