#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-sdk/log.h"
#include "odb-sdk/utf8.h"

static int
log_exit_error(
    const struct ast* ast,
    ast_id            exit,
    ast_id            first_loop,
    const char*       source_filename,
    struct db_source  source)
{
    int gutter;
    if (first_loop == -1)
    {
        log_flc_err(
            source_filename,
            source.text.data,
            ast->nodes[exit].info.location,
            "EXIT statement must be inside a loop.\n");
        log_excerpt_1(source.text.data, ast->nodes[exit].info.location, "");
    }
    else
    {
        struct utf8_span name = ast->nodes[first_loop].loop.name.len
                                    ? ast->nodes[first_loop].loop.name
                                : ast->nodes[first_loop].loop.implicit_name.len
                                    ? ast->nodes[first_loop].loop.implicit_name
                                    : empty_utf8_span();
        log_flc_err(
            source_filename,
            source.text.data,
            ast->nodes[exit].exit.name,
            "Unknown loop name referenced in EXIT statement.\n");
        gutter
            = log_excerpt_1(source.text.data, ast->nodes[exit].exit.name, "");
        if (name.len)
        {
            log_excerpt_help(
                gutter,
                "Did you mean {quote:%.*s}?\n",
                name.len,
                source.text.data + name.off);
            log_excerpt_1(source.text.data, name, "");
        }
    }
    return -1;
}

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

    ast_id first_loop = -1;
    ast_id loop = exit;
    while (1)
    {
        loop = ast_find_parent(ast, loop);
        if (loop == -1)
            return log_exit_error(
                ast, exit, first_loop, source_filename, source);

        if (ast->nodes[loop].info.node_type == AST_LOOP)
        {
            if (ast->nodes[exit].exit.name.len == 0
                || utf8_equal_span(
                    source.text.data,
                    ast->nodes[exit].exit.name,
                    ast->nodes[loop].loop.name)
                || utf8_equal_span(
                    source.text.data,
                    ast->nodes[exit].exit.name,
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

static const struct semantic_check* depends[]
    = {&semantic_expand_constant_declarations, NULL};

const struct semantic_check semantic_loop_exit = {check_loop_exit, depends};
