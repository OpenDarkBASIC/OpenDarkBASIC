#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"

static int
check_loop_names(
    struct ast*                ast,
    const struct plugin_list*  plugins,
    const struct cmd_list*     cmds,
    const struct symbol_table* symbols,
    const char*                source_filename,
    struct db_source           source)
{
    ast_id n;
    for (n = 0; n != ast->node_count; ++n)
    {
    }

    return 0;
}

static const struct semantic_check* depends[] = {NULL};

const struct semantic_check semantic_loop_name = {check_loop_names, depends};
