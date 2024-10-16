#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/messages/messages.h"
#include "odb-compiler/parser/db_source.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-util/log.h"
#include "odb-util/utf8.h"

static int
check_exit(
    const struct ast* ast,
    ast_id            exit,
    const char*       filename,
    const char*       source)
{
    ODBUTIL_DEBUG_ASSERT(exit > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, exit) == AST_LOOP_EXIT,
        log_semantic_err("type: %d\n", ast_node_type(ast, exit)));

    ast_id first_loop = -1;
    ast_id loop = exit;
    while (1)
    {
        loop = ast_find_parent(ast, loop);
        if (loop == -1)
        {
            if (first_loop == -1)
                return err_loop_exit_not_inside_loop(
                    ast, exit, filename, source);
            return err_loop_exit_unknown_name(ast, exit, first_loop, filename, source);
        }

        if (ast_node_type(ast, loop) == AST_LOOP)
        {
            if (ast->nodes[exit].loop_exit.name.len == 0
                || utf8_equal_span(
                    source,
                    ast->nodes[exit].loop_exit.name,
                    ast->nodes[loop].loop.name)
                || utf8_equal_span(
                    source,
                    ast->nodes[exit].loop_exit.name,
                    ast->nodes[loop].loop.implicit_name))
            {
                return 0;
            }

            if (first_loop == -1)
                first_loop = loop;
        }
    }
}

static int
check_loop_exit(
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
    ast_id            n;
    const struct ast* ast = tus[tu_id];
    const char*       filename = utf8_cstr(filenames[tu_id]);
    const char*       source = sources[tu_id].text.data;

    for (n = 0; n != ast_count(ast); ++n)
    {
        if (ast_node_type(ast, n) != AST_LOOP_EXIT)
            continue;

        if (check_exit(ast, n, filename, source) != 0)
            return -1;
    }

    return 0;
}

static const struct semantic_check* depends[] = {NULL};

const struct semantic_check semantic_loop_exit
    = {check_loop_exit, depends, "loop_exit"};
