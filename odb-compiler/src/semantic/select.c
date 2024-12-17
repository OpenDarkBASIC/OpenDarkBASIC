#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_integrity.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/messages/messages.h"
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
    union type       type = ast_type_info(*astp, expr);
    int32_t          scope = ast_scope(*astp, expr);
    struct utf8_span expr_loc = ast_loc(*astp, expr);
    struct utf8_span sel_loc = ast_loc(*astp, select);
    struct utf8_span hidden_name
        = {sel_loc.off, expr_loc.off - sel_loc.off + expr_loc.len};
    ast_id ident = ast_identifier(astp, hidden_name, TA_NONE, hidden_name);

    ODBUTIL_DEBUG_ASSERT(hidden_name.off == sel_loc.off, (void)0);
    ODBUTIL_DEBUG_ASSERT(hidden_name.len > 0, (void)0);

    /* Fill in type info and scope */
    (*astp)->nodes[ident].info.type_info = type;
    (*astp)->nodes[ident].info.scope_id = scope;

    return ident;
}

static ast_id
read_select_expr_into_hidden_var(struct ast** astp, ast_id select)
{
    int32_t scope = ast_scope(*astp, select);

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
    ast_id           select_expr = (*astp)->nodes[select].select.expr;
    union type       select_type = ast_type_info(*astp, select_expr);
    struct utf8_span select_loc = ast_loc(*astp, select_expr);
    ast_id           as = ast_as_type(astp, select_type, select_loc);

    ast_id decl1 = ast_var_decl(
        astp, ident, as, select_expr, SCOPE_LOCAL, loc, loc, loc);
    ast_id decl2 = (*astp)->nodes[decl1].var_decl1.var_decl2;

    /* Fill in type info and scope */
    (*astp)->nodes[decl1].info.type_info = select_type;
    (*astp)->nodes[decl2].info.type_info = select_type;
    (*astp)->nodes[as].info.type_info = select_type;
    ast_set_subtree_scope(*astp, decl1, scope);

    return decl1;
}

static int
convert_select_to_primitives(
    struct ast**   astp,
    ast_id         select,
    struct ospathc filename,
    const char*    source)
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

        /* Fill in type info and scope */
        (*astp)->nodes[branches].info.type_info = primitive_type(TYPE_VOID);
        (*astp)->nodes[var].info.type_info = ast_type_info(*astp, expr);
        (*astp)->nodes[op].info.type_info = primitive_type(TYPE_BOOL);
        (*astp)->nodes[cond].info.type_info = primitive_type(TYPE_VOID);
        (*astp)->nodes[no].info.type_info = primitive_type(TYPE_VOID);
        ast_set_subtree_scope(*astp, no, ast_scope(*astp, select));

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
report_duplicate_default_case(
    const struct ast* ast,
    ast_id            select,
    struct ospathc    filename,
    const char*       source)
{
    ast_id caselist, first_default_case;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, select) == AST_SELECT,
        log_err("type: %d\n", ast_node_type(ast, select)));
    caselist = ast->nodes[select].select.caselist;

    first_default_case = -1;
    for (; caselist > -1; caselist = ast->nodes[caselist].caselist.next)
    {
        ast_id default_case = ast->nodes[caselist].caselist.case_;
        ast_id expr = ast->nodes[default_case].case_.expr;
        if (expr > -1)
            continue;

        if (first_default_case == -1)
        {
            first_default_case = default_case;
            continue;
        }

        return err_select_duplicate_default(
            ast, default_case, first_default_case, filename, source);
    }

    return 0;
}

static int
select(
    struct ast**               tus,
    int                        tu_count,
    int                        tu_id,
    struct mutex**             tu_mutexes,
    const struct ospathc_list* filenames,
    struct utf8*               sources,
    const struct plugin_list*  plugins,
    const struct cmd_list*     cmds,
    const struct globals*      globals)
{
    ast_id         n;
    struct ast**   astp = &tus[tu_id];
    struct ast*    ast = *astp;
    struct ospathc filename = ospathc_list_get(filenames, tu_id);
    const char*    source = sources[tu_id].data;

    for (n = 0; n != ast_count(ast); ++n)
    {
        if (ast_node_type(*astp, n) != AST_SELECT)
            continue;

        if (report_duplicate_default_case(ast, n, filename, source) != 0)
            return -1;
        if (convert_select_to_primitives(astp, n, filename, source) != 0)
            return -1;
    }

    ast_gc(ast);
    return 0;
}

static const struct semantic_check* depends[] = {&semantic_type_check, NULL};

const struct semantic_check semantic_select = {select, depends, "select"};
