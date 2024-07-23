#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"

static int
translate_loop_for(
    struct ast*               ast,
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    const char*               source_filename,
    struct db_source          source)
{
    ast_id n;
    for (n = 0; n != ast->node_count; ++n)
    {
        if (ast->nodes[n].info.node_type != AST_LOOP)
            continue;
    }

    return 0;
}

static const struct semantic_check* depends[] = {&semantic_expand_constant_declarations, NULL};

const struct semantic_check semantic_loop_for = {translate_loop_for, depends};
