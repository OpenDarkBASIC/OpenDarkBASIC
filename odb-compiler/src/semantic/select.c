#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_integrity.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/messages/messages.h"
#include "odb-compiler/parser/db_source.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-util/log.h"

static ast_id
find_default_case(const struct ast* ast, ast_id caselist)
{
    ODBUTIL_DEBUG_ASSERT(
        caselist == -1 || ast_node_type(ast, caselist) == AST_CASELIST,
        log_err("type: %d\n", ast_node_type(ast, caselist)));

    for (; caselist > -1; caselist = ast->nodes[caselist].caselist.next)
    {
        ast_id case_ = ast->nodes[caselist].caselist.case_;
        if (ast->nodes[case_].case_.expr == -1)
            return case_;
    }

    return -1;
}

static ast_id
create_hidden_var_identifier(struct ast** astp, ast_id select)
{
    ast_id           expr = (*astp)->nodes[select].select.expr;
    struct utf8_span sel_loc = ast_loc(*astp, select);
    struct utf8_span expr_loc = ast_loc(*astp, expr);
    struct utf8_span hidden_name
        = {sel_loc.off, expr_loc.off - sel_loc.off + expr_loc.len};

    ODBUTIL_DEBUG_ASSERT(hidden_name.off == sel_loc.off, (void)0);
    ODBUTIL_DEBUG_ASSERT(hidden_name.len > 0, (void)0);
    return ast_identifier(astp, hidden_name, TA_NONE, hidden_name);
}

static ast_id
read_select_expr_into_hidden_var(struct ast** astp, ast_id select)
{
    /* The select expression is compared against each case expression, but we
     * must be careful to only evaluate it once. We do this by storing the
     * result in a "hidden" variable. As of this writing, the compiler doesn't
     * officially support hidden variables, in fact, it relies on variables
     * having unique names. We can however create a unique name by including the
     * "select" statement along with the expression. Because it contains a
     * space, it'll be impossible to accidentally shadow by the user */
    ast_id           ident = create_hidden_var_identifier(astp, select);
    struct utf8_span loc = ast_loc(*astp, ident);

    /* The hidden variable's type must be equal to that of the select expression
     */
    ast_id as = ast_as_auto(astp, loc);
    ast_id select_expr = (*astp)->nodes[select].select.expr;

    return ast_var_decl(
        astp, ident, as, select_expr, SCOPE_LOCAL, loc, loc, loc);
}

static int
convert_select_to_primitives(
    struct ast** astp, ast_id select, const char* filename, const char* source)
{
    ast_id select_result_decl, caselist, default_case, no, block;

    caselist = (*astp)->nodes[select].select.caselist;
    ODBUTIL_DEBUG_ASSERT(
        caselist == -1 || ast_node_type(*astp, caselist) == AST_CASELIST,
        log_err("type: %d\n", ast_node_type(*astp, caselist)));

    default_case = find_default_case(*astp, caselist);
    ODBUTIL_DEBUG_ASSERT(
        default_case == -1 || ast_node_type(*astp, default_case) == AST_CASE,
        log_err("type: %d\n", ast_node_type(*astp, default_case)));

    select_result_decl = read_select_expr_into_hidden_var(astp, select);

    no = default_case > -1 ? (*astp)->nodes[default_case].case_.body : -1;
    for (; caselist > -1; caselist = (*astp)->nodes[caselist].caselist.next)
    {
        struct utf8_span loc;
        ast_id           case_, expr, yes, branches, var_ident, var, op, cond;

        case_ = (*astp)->nodes[caselist].caselist.case_;
        if (case_ == default_case)
            continue;

        loc = ast_loc(*astp, caselist);
        expr = (*astp)->nodes[case_].case_.expr;
        yes = (*astp)->nodes[case_].case_.body;
        branches = ast_cond_branches(astp, yes, no, loc);
        var_ident = create_hidden_var_identifier(astp, select);
        var = ast_var_read(astp, var_ident, ast_loc(*astp, var_ident));
        op = ast_binop(
            astp,
            BINOP_EQUAL,
            var,
            expr,
            ast_loc(*astp, expr),
            ast_loc(*astp, case_));
        cond = ast_cond(astp, op, branches, ast_loc(*astp, expr));
        no = ast_block(astp, cond, loc);

        /* Unlink from old tree */
        (*astp)->nodes[case_].case_.expr = -1;
        (*astp)->nodes[case_].case_.body = -1;
    }

    /* Unlink from old tree */
    (*astp)->nodes[select].select.expr = -1;
    if (default_case > -1)
        (*astp)->nodes[default_case].case_.body = -1;

    block = ast_find_parent(*astp, select);
    ODBUTIL_DEBUG_ASSERT(block > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(*astp, block) == AST_BLOCK,
        log_err("type: %d\n", ast_node_type(*astp, block)));

    (*astp)->nodes[block].block.stmt = select_result_decl;
    if (no > -1)
        ast_block_append(*astp, block, no);

    ast_delete_tree(*astp, select);

    return 0;
}

static int
select(
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
    ast_id       n;
    struct ast** astp = &tus[tu_id];
    struct ast*  ast = *astp;
    const char*  filename = utf8_cstr(filenames[tu_id]);
    const char*  source = sources[tu_id].text.data;

    for (n = 0; n != ast_count(ast); ++n)
    {
        if (ast_node_type(*astp, n) != AST_SELECT)
            continue;

        if (convert_select_to_primitives(astp, n, filename, source) != 0)
            return -1;
    }

    ast_gc(ast);
    return 0;
}

static const struct semantic_check* depends[] = {NULL};

const struct semantic_check semantic_select = {select, depends, "select"};
