#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/messages/messages.h"
#include "odb-compiler/parser/db_source.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-util/log.h"
#include "odb-util/utf8.h"

static void
find_parent_loop_with_same_implicit_name(
    const struct ast* ast,
    ast_id            exit,
    ast_id            loop,
    const char*       filename,
    const char*       source)
{
    struct utf8_span name = ast->nodes[loop].loop1.implicit_name;
    while ((loop = ast_find_parent(ast, loop)) > -1)
    {
        /* Can't cross function boundaries */
        if (ast_node_type(ast, loop) == AST_FUNC1)
            break;

        if (ast_node_type(ast, loop) == AST_LOOP1)
        {
            struct utf8_span outer_name = ast->nodes[loop].loop1.implicit_name;
            if (outer_name.len && utf8_equal_span(source, name, outer_name))
            {
                warn_loop_exit_ambiguous_name(
                    ast, exit, name, outer_name, filename, source);
                return;
            }
        }
    }
}

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
        log_err("type: %d\n", ast_node_type(ast, exit)));

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
            return err_loop_exit_unknown_name(
                ast, exit, first_loop, filename, source);
        }

        if (ast_node_type(ast, loop) == AST_LOOP1)
        {
            struct utf8_span exit_name = ast->nodes[exit].loop_exit.name;
            struct utf8_span loop_name = ast->nodes[loop].loop1.name;
            struct utf8_span loop_implicit_name
                = ast->nodes[loop].loop1.implicit_name;
            if (exit_name.len == 0
                || utf8_equal_span(source, exit_name, loop_name)
                || utf8_equal_span(source, exit_name, loop_implicit_name))
            {
                if (exit_name.len > 0 && loop_implicit_name.len > 0)
                    find_parent_loop_with_same_implicit_name(
                        ast, exit, loop, filename, source);
                return 0;
            }

            if (first_loop == -1)
                first_loop = loop;
        }
    }
}

static int
check_loop_exit(
    struct ast**              tus,
    int                       tu_count,
    int                       tu_id,
    struct mutex**            tu_mutexes,
    const struct utf8*        filenames,
    const struct db_source*   sources,
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    const struct globals*     globals)
{
    ast_id            n;
    const struct ast* ast = tus[tu_id];
    const char*       filename = utf8_cstr(filenames[tu_id]);
    const char*       source = sources[tu_id].text.data;

    for (n = 0; n != ast_count(ast); ++n)
        if (ast_node_type(ast, n) == AST_LOOP_EXIT)
            if (check_exit(ast, n, filename, source) != 0)
                return -1;

    return 0;
}

static const struct semantic_check* depends[] = {NULL};

const struct semantic_check semantic_loop_exit
    = {check_loop_exit, depends, "loop_exit"};
