#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/messages/messages.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-util/log.h"

/* TODO: This code sucks. It was written to get for loops working as fast as
 * possible with no error checking */

static int
get_loop_var_lvalue(const struct ast* ast, ast_id loop)
{
    ast_id loop2, post_body, step_stmt;

    loop2 = ast->nodes[loop].loop1.loop2;
    ODBUTIL_DEBUG_ASSERT(loop2 > -1, (void)0);

    post_body = ast->nodes[loop2].loop2.post_body;
    ODBUTIL_DEBUG_ASSERT(post_body > -1, (void)0);

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, post_body) == AST_BLOCK,
        log_err("type: %d\n", ast_node_type(ast, post_body)));
    step_stmt = ast->nodes[post_body].block.stmt;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, step_stmt) == AST_ASSIGNMENT,
        log_err("type: %d\n", ast_node_type(ast, step_stmt)));
    return ast->nodes[step_stmt].assignment.lvalue;
}

static ast_id
create_step_block(struct ast** astp, ast_id loop, ast_id cont)
{
    if ((*astp)->nodes[cont].cont.step > -1)
    {
        ast_id step_expr = (*astp)->nodes[cont].cont.step;
        ast_id loop_lvalue = get_loop_var_lvalue(*astp, loop);
        ast_id inc_var = ast_dup_lvalue(astp, loop_lvalue);
        ast_id inc_stmt
            = ast_inc_step(astp, inc_var, step_expr, ast_loc(*astp, step_expr));

        return ast_block(astp, inc_stmt, ast_loc(*astp, step_expr));
    }
    else
    {
        ast_id loop_body = (*astp)->nodes[loop].loop1.loop2;
        ast_id post_body = (*astp)->nodes[loop_body].loop2.post_body;
        ODBUTIL_DEBUG_ASSERT(post_body > -1, (void)0);
        return ast_dup_subtree(astp, post_body);
    }
}

static int
check_cont(
    const struct ast* ast,
    ast_id            cont,
    struct ospathc    filename,
    const char*       source)
{
    ODBUTIL_DEBUG_ASSERT(cont > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, cont) == AST_LOOP_CONT,
        log_err("type: %d\n", ast_node_type(ast, cont)));

    ast_id first_loop = -1;
    ast_id loop = cont;
    while (1)
    {
        loop = ast_find_parent(ast, loop);
        if (loop == -1)
            return err_loop_cont(ast, cont, first_loop, filename, source);

        if (ast_node_type(ast, loop) == AST_LOOP1)
        {
            if (ast->nodes[cont].cont.name.len == 0
                || utf8_equal_span(
                    source,
                    ast->nodes[cont].cont.name,
                    ast->nodes[loop].loop1.name)
                || utf8_equal_span(
                    source,
                    ast->nodes[cont].cont.name,
                    ast->nodes[loop].loop1.implicit_name))
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
    const struct ospathc_list* filenames,
    struct utf8*               sources,
    const struct plugin_list*  plugins,
    const struct cmd_list*     cmds,
    const struct udt_storage*  udts,
    const struct globals*      globals)
{
    ast_id         n, loop;
    struct ast**   astp = &tus[tu_id];
    struct ospathc filename = ospathc_list_get(filenames, tu_id);
    const char*    source = sources[tu_id].data;

    for (n = 0; n != ast_count(*astp); ++n)
    {
        if (ast_node_type(*astp, n) != AST_LOOP_CONT)
            continue;

        loop = check_cont(*astp, n, filename, source);
        if (loop == -1)
            return -1;

        (*astp)->nodes[n].cont.step = create_step_block(astp, loop, n);
        if ((*astp)->nodes[n].cont.step == -1)
            return -1;
    }

    return 0;
}

static const struct semantic_check* depends[]
    = {&semantic_loop_for, /* Need loop.post_body to resolve cont.step */
       NULL};

const struct semantic_check semantic_loop_cont
    = {check_loop_cont, depends, "loop_cont"};
