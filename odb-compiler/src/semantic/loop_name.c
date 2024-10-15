#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"

static int
check_loop_names(
    struct ast**               tus,
    int                        tu_count,
    int                        tu_id,
    struct mutex**             tu_mutexes,
    const struct utf8*         filenames,
    const struct db_source*    sources,
    const struct plugin_list*  plugins,
    const struct cmd_list*     cmds,
    const struct symbol_table* symbols)
{
    ast_id      n;
    struct ast* ast = tus[tu_id];
    for (n = 0; n != ast->count; ++n)
    {
    }

    return 0;
}

static const struct semantic_check* depends[] = {NULL};

const struct semantic_check semantic_loop_name = {check_loop_names, depends};
