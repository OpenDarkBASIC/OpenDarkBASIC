#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-sdk/log.h"

static int
check_exit(
    struct ast*      ast,
    ast_id           exit,
    const char*      source_filename,
    struct db_source source)
{
    ODBSDK_DEBUG_ASSERT(exit > -1, (void)0);
    ODBSDK_DEBUG_ASSERT(
        ast->nodes[exit].info.node_type == AST_LOOP_EXIT,
        log_semantic_err("type: %d\n", ast->nodes[exit].info.node_type));

    ast_id loop = exit;
    while (1)
    {
        loop = ast_find_parent(ast, loop);
        if (loop == -1)
        {
            log_flc_err(
                source_filename,
                source.text.data,
                ast->nodes[exit].info.location,
                "EXIT statement must be inside a loop.\n");
            log_excerpt_1(source.text.data, ast->nodes[exit].info.location, "");
            return -1;
        }

        if (ast->nodes[loop].info.node_type == AST_LOOP)
            break;
    }

    return 0;
}

static int
check_loop_exit(
    struct ast*               ast,
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    const char*               source_filename,
    struct db_source          source)
{
    ast_id n;
    for (n = 0; n != ast->node_count; ++n)
    {
        if (ast->nodes[n].info.node_type != AST_LOOP_EXIT)
            continue;

        if (check_exit(ast, n, source_filename, source) != 0)
            return -1;
    }

    return 0;
}

static const struct semantic_check* depends[] = {NULL};

const struct semantic_check semantic_loop_exit = {check_loop_exit, depends};
