#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/parser/db_source.h"
#include "odb-compiler/semantic/semantic.h"

static int
get_loop_var(const struct ast* ast, ast_id loop)
{
    ODBUTIL_DEBUG_ASSERT(ast->nodes[loop].loop.post_body > -1, (void)0);
    ast_id post = ast->nodes[loop].loop.post_body;
    ast_id step_stmt = ast->nodes[post].block.stmt;
    return ast->nodes[step_stmt].assignment.lvalue;
}

static void
create_step_block(struct ast* ast, ast_id loop, ast_id cont)
{
    if (ast->nodes[cont].cont.step > -1)
    {
        ast_id step_expr = ast->nodes[cont].cont.step;
        ast_id loop_var = get_loop_var(ast, loop);
        ast_id inc_var = ast_dup_lvalue(ast, loop_var);
        ast_id inc_stmt = ast_inc_step(
            ast, inc_var, step_expr, ast->nodes[step_expr].info.location);
        ast->nodes[cont].cont.step
            = ast_block(ast, inc_stmt, ast->nodes[step_expr].info.location);
    }
    else
    {
        ODBUTIL_DEBUG_ASSERT(ast->nodes[loop].loop.post_body > -1, (void)0);
        ast->nodes[cont].cont.step = ast->nodes[loop].loop.post_body;
    }
}

static int
log_cont_error(
    const struct ast* ast,
    ast_id            cont,
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
            ast->nodes[cont].info.location,
            "CONTINUE statement must be inside a loop.\n");
        log_excerpt_1(source_text, ast->nodes[cont].info.location, "");
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
            ast->nodes[cont].cont.name,
            "Unknown loop name referenced in CONTINUE statement.\n");
        gutter = log_excerpt_1(source_text, ast->nodes[cont].cont.name, "");
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
check_cont(
    struct ast* ast,
    ast_id      cont,
    const char* source_filename,
    const char* source_text)
{
    ODBUTIL_DEBUG_ASSERT(cont > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast->nodes[cont].info.node_type == AST_LOOP_CONT,
        log_semantic_err("type: %d\n", ast->nodes[cont].info.node_type));

    ast_id first_loop = -1;
    ast_id loop = cont;
    while (1)
    {
        loop = ast_find_parent(ast, loop);
        if (loop == -1)
            return log_cont_error(
                ast, cont, first_loop, source_filename, source_text);

        if (ast->nodes[loop].info.node_type == AST_LOOP)
        {
            if (ast->nodes[cont].cont.name.len == 0
                || utf8_equal_span(
                    source_text,
                    ast->nodes[cont].cont.name,
                    ast->nodes[loop].loop.name)
                || utf8_equal_span(
                    source_text,
                    ast->nodes[cont].cont.name,
                    ast->nodes[loop].loop.implicit_name))
            {
                return loop;
            }

            if (first_loop == -1)
                first_loop = loop;
        }
    }
}

static int
check_loop_cont(
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
    ast_id      n, loop;
    struct ast* ast = &asts[asts_id];
    const char* filename = filenames[asts_id];
    const char* source = sources[asts_id].text.data;

    for (n = 0; n != ast->node_count; ++n)
    {
        if (ast->nodes[n].info.node_type != AST_LOOP_CONT)
            continue;

        loop = check_cont(ast, n, filename, source);
        if (loop == -1)
            return -1;

        create_step_block(ast, loop, n);
    }

    return 0;
}

static const struct semantic_check* depends[]
    = {&semantic_loop_for, /* Need loop.post_body to resolve cont.step */
       NULL};

const struct semantic_check semantic_loop_cont = {check_loop_cont, depends};
