#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/parser/db_source.h"
#include "odb-compiler/semantic/semantic.h"

/* TODO: This code sucks. It was written to get for loops working as fast as
 * possible with no error checking */

static int
get_loop_var(const struct ast* ast, ast_id loop)
{
    ast_id loop_body, post_body, step_stmt;

    loop_body = ast->nodes[loop].loop.loop_body;
    ODBUTIL_DEBUG_ASSERT(loop_body > -1, (void)0);

    post_body = ast->nodes[loop_body].loop_body.post_body;
    ODBUTIL_DEBUG_ASSERT(post_body > -1, (void)0);

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, post_body) == AST_BLOCK,
        log_semantic_err("type: %d\n", ast_node_type(ast, post_body)));
    step_stmt = ast->nodes[post_body].block.stmt;
    return ast->nodes[step_stmt].assignment.lvalue;
}

static void
create_step_block(struct ast** astp, ast_id loop, ast_id cont)
{
    if ((*astp)->nodes[cont].cont.step > -1)
    {
        ast_id step_expr = (*astp)->nodes[cont].cont.step;
        ast_id loop_var = get_loop_var(*astp, loop);
        ast_id inc_var = ast_dup_lvalue(astp, loop_var);
        ast_id inc_stmt = ast_inc_step(
            astp, inc_var, step_expr, (*astp)->nodes[step_expr].info.location);
        (*astp)->nodes[cont].cont.step = ast_block(
            astp, inc_stmt, (*astp)->nodes[step_expr].info.location);
    }
    else
    {
        ast_id loop_body = (*astp)->nodes[loop].loop.loop_body;
        ast_id post_body = (*astp)->nodes[loop_body].loop_body.post_body;
        ODBUTIL_DEBUG_ASSERT(post_body > -1, (void)0);
        (*astp)->nodes[cont].cont.step = post_body;
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
    const struct ast* ast,
    ast_id            cont,
    const char*       source_filename,
    const char*       source_text)
{
    ODBUTIL_DEBUG_ASSERT(cont > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, cont) == AST_LOOP_CONT,
        log_semantic_err("type: %d\n", ast_node_type(ast, cont)));

    ast_id first_loop = -1;
    ast_id loop = cont;
    while (1)
    {
        loop = ast_find_parent(ast, loop);
        if (loop == -1)
            return log_cont_error(
                ast, cont, first_loop, source_filename, source_text);

        if (ast_node_type(ast, loop) == AST_LOOP)
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
    ast_id       n, loop;
    struct ast** astp = &tus[tu_id];
    struct ast*  ast = *astp;
    const char*  filename = utf8_cstr(filenames[tu_id]);
    const char*  source = sources[tu_id].text.data;

    for (n = 0; n != ast_count(ast); ++n)
    {
        if (ast_node_type(ast, n) != AST_LOOP_CONT)
            continue;

        loop = check_cont(ast, n, filename, source);
        if (loop == -1)
            return -1;

        create_step_block(astp, loop, n);
        ast = *astp;
    }

    return 0;
}

static const struct semantic_check* depends[]
    = {&semantic_loop_for, /* Need loop.post_body to resolve cont.step */
       NULL};

const struct semantic_check semantic_loop_cont = {check_loop_cont, depends, "loop_cont"};
