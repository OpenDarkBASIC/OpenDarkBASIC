#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/parser/db_source.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-util/log.h"
#include "odb-util/utf8.h"

static int
log_exit_error(
    const struct ast* ast,
    ast_id            exit,
    ast_id            first_loop,
    const char*       source_filename,
    const char*       source_text)
{
    int gutter;
    if (first_loop == -1)
    {
        log_flc_err(
            source_filename,
            source_text,
            ast->nodes[exit].info.location,
            "EXIT statement must be inside a loop.\n");
        log_excerpt_1(source_text, ast->nodes[exit].info.location, "");
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
            source_text,
            ast->nodes[exit].loop_exit.name,
            "Unknown loop name referenced in EXIT statement.\n");
        gutter
            = log_excerpt_1(source_text, ast->nodes[exit].loop_exit.name, "");
        if (name.len)
        {
            log_excerpt_help(
                gutter,
                "Did you mean {quote:%.*s}?\n",
                name.len,
                source_text + name.off);
            log_excerpt_1(source_text, name, "");
        }
    }
    return -1;
}

static int
check_exit(
    const struct ast* ast,
    ast_id            exit,
    const char*       source_filename,
    const char*       source_text)
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
            return log_exit_error(
                ast, exit, first_loop, source_filename, source_text);

        if (ast_node_type(ast, loop) == AST_LOOP)
        {
            if (ast->nodes[exit].loop_exit.name.len == 0
                || utf8_equal_span(
                    source_text,
                    ast->nodes[exit].loop_exit.name,
                    ast->nodes[loop].loop.name)
                || utf8_equal_span(
                    source_text,
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

const struct semantic_check semantic_loop_exit = {check_loop_exit, depends};
