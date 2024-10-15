#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"

static int
check_loop_names(
    struct ast*                asts,
    int                        asts_count,
    int                        asts_id,
    struct mutex**             asts_mutex,
    const char**               filenames,
    const struct db_source*    sources,
    const struct plugin_list*  plugins,
    const struct cmd_list*     cmds,
    const struct symbol_table* symbols)
{
    ast_id      n;
    struct ast* ast = &asts[asts_id];
    for (n = 0; n != ast->node_count; ++n)
    {
    }

    return 0;
}

static const struct semantic_check* depends[] = {NULL};

const struct semantic_check semantic_loop_name = {check_loop_names, depends};
