#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/ast/ast_integrity.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/parser/db_source.h"
#include "odb-compiler/semantic/semantic.h"

enum expr_type
{
    EXPR_TYPE_UNKNOWN,
    EXPR_TYPE_INTEGER,
    EXPR_TYPE_FLOAT,
};

union expr_value
{
    int64_t i;
    double  f;
};

static enum expr_type
eval_constant_expr(const struct ast* ast, ast_id n, union expr_value* value)
{
    switch (ast->nodes[n].info.node_type)
    {
        case AST_GC: break;
        case AST_BLOCK: break;
        case AST_END: break;
        case AST_ARGLIST: break;
        case AST_PARAMLIST: break;
        case AST_COMMAND: break;
        case AST_ASSIGNMENT: break;
        case AST_IDENTIFIER: break;
        case AST_BINOP: break;
        case AST_UNOP: break;
        case AST_COND: break;
        case AST_COND_BRANCH: break;
        case AST_LOOP: break;
        case AST_LOOP_BODY: break;
        case AST_LOOP_FOR1: break;
        case AST_LOOP_FOR2: break;
        case AST_LOOP_FOR3: break;
        case AST_LOOP_CONT: break;
        case AST_LOOP_EXIT: break;
        case AST_FUNC_TEMPLATE: break;
        case AST_FUNC: break;
        case AST_FUNC_DECL: break;
        case AST_FUNC_DEF: break;
        case AST_FUNC_EXIT: break;
        case AST_FUNC_OR_CONTAINER_REF: break;
        case AST_FUNC_CALL: break;

        case AST_BOOLEAN_LITERAL:
            value->i = ast->nodes[n].boolean_literal.is_true ? 1 : 0;
            return EXPR_TYPE_INTEGER;
        case AST_BYTE_LITERAL:
            value->i = ast->nodes[n].byte_literal.value;
            return EXPR_TYPE_INTEGER;
        case AST_WORD_LITERAL:
            value->i = ast->nodes[n].word_literal.value;
            return EXPR_TYPE_INTEGER;
        case AST_DWORD_LITERAL:
            value->i = ast->nodes[n].dword_literal.value;
            return EXPR_TYPE_INTEGER;
        case AST_INTEGER_LITERAL:
            value->i = ast->nodes[n].integer_literal.value;
            return EXPR_TYPE_INTEGER;
        case AST_DOUBLE_INTEGER_LITERAL:
            value->i = ast->nodes[n].double_integer_literal.value;
            return EXPR_TYPE_INTEGER;

        case AST_FLOAT_LITERAL:
            value->f = ast->nodes[n].float_literal.value;
            return EXPR_TYPE_FLOAT;
        case AST_DOUBLE_LITERAL:
            value->f = ast->nodes[n].double_literal.value;
            return EXPR_TYPE_FLOAT;

        case AST_STRING_LITERAL: break;
        case AST_CAST: break;
        case AST_SCOPE: break;
    }

    return EXPR_TYPE_UNKNOWN;
}

static int
loop_direction(
    enum expr_type   begin_type,
    union expr_value begin_val,
    enum expr_type   end_type,
    union expr_value end_val)
{
    if (begin_type == EXPR_TYPE_INTEGER && end_type == EXPR_TYPE_INTEGER)
        return begin_val.i <= end_val.i ? 1 : -1;
    if (begin_type == EXPR_TYPE_FLOAT && end_type == EXPR_TYPE_FLOAT)
        return begin_val.f <= end_val.f ? 1 : -1;
    if (begin_type == EXPR_TYPE_INTEGER && end_type == EXPR_TYPE_FLOAT)
        return begin_val.i <= end_val.f ? 1 : -1;
    if (begin_type == EXPR_TYPE_FLOAT && end_type == EXPR_TYPE_INTEGER)
        return begin_val.f <= end_val.i ? 1 : -1;
    return 0;
}

static int
step_direction(enum expr_type step_type, union expr_value step_val)
{
    if (step_type == EXPR_TYPE_INTEGER)
        return step_val.i >= 0 ? 1 : -1;
    if (step_type == EXPR_TYPE_FLOAT)
        return step_val.f >= 0 ? 1 : -1;
    return 0;
}

static ast_id
create_exit_stmt(
    struct ast**     astp,
    ast_id           begin,
    ast_id           end,
    ast_id           step,
    ast_id           loop_var,
    struct utf8_span location,
    const char*      source_filename,
    const char*      source_text)
{
    union expr_value begin_val, end_val, step_val = {0};
    enum expr_type   begin_type = eval_constant_expr(*astp, begin, &begin_val);
    enum expr_type   end_type = eval_constant_expr(*astp, end, &end_val);
    enum expr_type   step_type = step > -1
                                     ? eval_constant_expr(*astp, step, &step_val)
                                     : EXPR_TYPE_UNKNOWN;
    int loop_dir = loop_direction(begin_type, begin_val, end_type, end_val);
    int step_dir = step_direction(step_type, step_val);
    enum binop_type cmp_op
        = step_dir < 0 ? BINOP_LESS_THAN : BINOP_GREATER_THAN;

    if (step > -1 && step_type == EXPR_TYPE_UNKNOWN
        && (begin_type == EXPR_TYPE_UNKNOWN || end_type == EXPR_TYPE_UNKNOWN))
    {
        int              gutter;
        struct utf8_span loc1 = utf8_span_union(
            (*astp)->nodes[begin].info.location,
            (*astp)->nodes[end].info.location);
        struct utf8_span     loc2 = (*astp)->nodes[step].info.location;
        struct log_highlight hl[]
            = {{"", "", loc1, LOG_HIGHLIGHT, LOG_MARKERS, 0},
               {"", "", loc2, LOG_HIGHLIGHT, LOG_MARKERS, 0},
               LOG_HIGHLIGHT_SENTINAL};
        log_flc_err(
            source_filename,
            source_text,
            loc1,
            "Unable to determine direction of for-loop.\n");
        gutter = log_excerpt(source_text, hl);
        log_excerpt_note(
            gutter,
            "The direction a for-loop counts must be known at compile-time, "
            "because the exit condition depends on it. You can either make the "
            "STEP value a constant, or make both the start and end values "
            "constants.\n");
        return -1;
    }

    if (step == -1
        && (begin_type == EXPR_TYPE_UNKNOWN || end_type == EXPR_TYPE_UNKNOWN))
    {
        int              gutter;
        struct utf8_span loc = utf8_span_union(
            (*astp)->nodes[begin].info.location,
            (*astp)->nodes[end].info.location);
        utf8_idx             ins = loc.off + loc.len;
        struct log_highlight hl_step_forwards[]
            = {{" STEP 1", "", {ins, 7}, LOG_INSERT, LOG_MARKERS, 0},
               LOG_HIGHLIGHT_SENTINAL};
        struct log_highlight hl_step_backwards[]
            = {{" STEP -1", "", {ins, 8}, LOG_INSERT, LOG_MARKERS, 0},
               LOG_HIGHLIGHT_SENTINAL};
        log_flc_warn(
            source_filename,
            source_text,
            loc,
            "For-loop direction may be incorrect.\n");
        gutter = log_excerpt_1(source_text, loc, "");
        log_excerpt_help(
            gutter,
            "If no STEP is specified, it will default to 1. You can silence "
            "this warning by making the STEP explicit:\n");
        log_excerpt(source_text, hl_step_forwards);
        log_excerpt(source_text, hl_step_backwards);
    }

    if (begin_type != EXPR_TYPE_UNKNOWN && end_type != EXPR_TYPE_UNKNOWN
        && step_type != EXPR_TYPE_UNKNOWN && loop_dir != step_dir)
    {
        struct utf8_span loc1 = utf8_span_union(
            (*astp)->nodes[begin].info.location,
            (*astp)->nodes[end].info.location);
        struct utf8_span     loc2 = (*astp)->nodes[step].info.location;
        struct log_highlight hl[]
            = {{"", "", loc1, LOG_HIGHLIGHT, LOG_MARKERS, 0},
               {"", "", loc2, LOG_HIGHLIGHT, LOG_MARKERS, 0},
               LOG_HIGHLIGHT_SENTINAL};
        log_flc_warn(
            source_filename,
            source_text,
            loc1,
            "For-loop does nothing, because it STEPs in the wrong "
            "direction.\n");
        log_excerpt(source_text, hl);
    }

    if (begin_type != EXPR_TYPE_UNKNOWN && end_type != EXPR_TYPE_UNKNOWN
        && step == -1 && loop_dir < 0)
    {
        int              gutter;
        struct utf8_span loc = utf8_span_union(
            (*astp)->nodes[begin].info.location,
            (*astp)->nodes[end].info.location);
        utf8_idx             ins = loc.off + loc.len;
        struct log_highlight hl[]
            = {{" STEP -1", "", {ins, 8}, LOG_INSERT, LOG_MARKERS, 0},
               LOG_HIGHLIGHT_SENTINAL};
        log_flc_warn(
            source_filename,
            source_text,
            loc,
            "For-loop does nothing, because it STEPs in the wrong "
            "direction.\n");
        gutter = log_excerpt_1(source_text, loc, "");
        log_excerpt_help(
            gutter,
            "If no STEP is specified, it will default to 1. You can make a "
            "loop count backwards as follows:\n");
        log_excerpt(source_text, hl);
    }

    struct utf8_span begin_loc = (*astp)->nodes[begin].info.location;
    struct utf8_span end_loc = (*astp)->nodes[end].info.location;

    ast_id exit = ast_loop_exit(astp, empty_utf8_span(), begin_loc);
    ast_id exit_cond_block = ast_block(astp, exit, begin_loc);
    ast_id exit_cond_branch
        = ast_cond_branch(astp, exit_cond_block, -1, begin_loc);
    ast_id exit_var = ast_dup_lvalue(astp, loop_var);
    ast_id exit_expr
        = ast_binop(astp, cmp_op, exit_var, end, begin_loc, end_loc);
    ast_id exit_stmt = ast_cond(astp, exit_expr, exit_cond_branch, begin_loc);

    return exit_stmt;
}

static void
compare_next_stmt_with_loop_var(
    struct ast* ast,
    ast_id      next,
    ast_id      loop_var,
    const char* source_filename,
    const char* source_text)
{
    ODBUTIL_DEBUG_ASSERT(next > -1, log_err("ast", "next: %d\n", next));
    if (!ast_trees_equal(source_text, ast, loop_var, next))
    {
        int gutter;
        log_flc_warn(
            source_filename,
            source_text,
            ast->nodes[next].info.location,
            "Loop variable in next statement is different from the one "
            "used in the for-loop statement.\n");
        gutter = log_excerpt_1(source_text, ast->nodes[next].info.location, "");
        log_excerpt_note(gutter, "Loop variable declared here:\n");
        log_excerpt_1(source_text, ast->nodes[loop_var].info.location, "");
    }
}

static ast_id
convert_for_loop_to_primitives(
    struct ast** astp,
    ast_id       loop,
    const char*  source_filename,
    const char*  source_text)
{
    struct utf8_span loop_loc;
    ast_id           for1, for2, for3, loop_body, body, post_body;
    ast_id           init, loop_var, begin, end, step, next;

    loop_loc = (*astp)->nodes[loop].info.location;

    for1 = (*astp)->nodes[loop].loop.loop_for1;
    ODBUTIL_DEBUG_ASSERT(for1 > -1, log_semantic_err("loop_for1: %d\n", for1));
    for2 = (*astp)->nodes[for1].loop_for1.loop_for2;
    ODBUTIL_DEBUG_ASSERT(for2 > -1, log_semantic_err("loop_for2: %d\n", for2));
    for3 = (*astp)->nodes[for2].loop_for2.loop_for3;
    ODBUTIL_DEBUG_ASSERT(for3 > -1, log_semantic_err("loop_for3: %d\n", for3));

    init = (*astp)->nodes[for1].loop_for1.init;
    end = (*astp)->nodes[for2].loop_for2.end;
    step = (*astp)->nodes[for3].loop_for3.step;
    next = (*astp)->nodes[for3].loop_for3.next;
    ODBUTIL_DEBUG_ASSERT(init > -1, log_semantic_err("init: %d\n", init));
    ODBUTIL_DEBUG_ASSERT(end > -1, log_semantic_err("end: %d\n", end));
    ODBUTIL_DEBUG_ASSERT(
        (*astp)->nodes[init].info.node_type == AST_ASSIGNMENT,
        log_semantic_err("type: %d\n", (*astp)->nodes[init].info.node_type));
    loop_var = (*astp)->nodes[init].assignment.lvalue;
    begin = (*astp)->nodes[init].assignment.expr;

    loop_body = (*astp)->nodes[loop].loop.loop_body;
    ODBUTIL_DEBUG_ASSERT(
        loop_body > -1, log_semantic_err("loop_body: %d\n", loop_body));
    body = (*astp)->nodes[loop_body].loop_body.body;
    post_body = (*astp)->nodes[loop_body].loop_body.post_body;
    ODBUTIL_DEBUG_ASSERT(
        post_body == -1, log_semantic_err("post_body: %d\n", post_body));

    /* Check next expr if one exists, then delete it as it has no meaning.
     * Historyically, this was used to be more explicit about which loop the
     * "next" belongs to. */
    if (next > -1)
    {
        compare_next_stmt_with_loop_var(
            (*astp), next, loop_var, source_filename, source_text);
        (*astp)->nodes[for3].loop_for3.next = -1;
        ast_delete_tree(*astp, next);
    }

    /* Creates the exit condition as a statement. This gets inserted at the very
     * beginning of the loop's body. */
    ast_id exit_stmt = create_exit_stmt(
        astp,
        begin,
        end,
        step,
        loop_var,
        loop_loc,
        source_filename,
        source_text);
    if (exit_stmt < 0)
        return -1;
    /* The above function "steals" the end node */
    (*astp)->nodes[for2].loop_for2.end = -1;

    /* Create the post-increment statement. This gets inserted into the loop's
     * "post_body" property, which will effectively insert it at the end of the
     * body later on */
    ast_id inc_var = ast_dup_lvalue(astp, loop_var);
    ast_id inc_stmt
        = step > -1
              ? ast_inc_step(
                    astp, inc_var, step, (*astp)->nodes[step].info.location)
              : ast_inc(astp, inc_var, (*astp)->nodes[inc_var].info.location);

    /* Insert the exit condition into the beginning of the loop body */
    ast_id exit_block = ast_block(astp, exit_stmt, loop_loc);
    (*astp)->nodes[exit_block].block.next = body;
    (*astp)->nodes[loop_body].loop_body.body = exit_block;

    /* Insert post-increment into the end, and make sure to remove it as a child
     * from the loop_for nodes */
    ODBUTIL_DEBUG_ASSERT(
        post_body == -1, log_semantic_err("post_body: %d\n", post_body));
    ast_id inc_block = ast_block(astp, inc_stmt, loop_loc);
    (*astp)->nodes[loop_body].loop_body.post_body = inc_block;
    (*astp)->nodes[for3].loop_for3.step = -1;

    /* Loop variable initialization statement is inserted outside of the loop */
    ast_id loop_block = ast_find_parent((*astp), loop);
    ODBUTIL_DEBUG_ASSERT(
        loop_block > -1, log_semantic_err("loop_block: %d\n", loop_block));
    ODBUTIL_DEBUG_ASSERT(
        (*astp)->nodes[loop_block].info.node_type == AST_BLOCK,
        log_semantic_err(
            "loop_block: %d\n", (*astp)->nodes[loop_block].info.node_type));
    ast_id init_block = ast_block_append_stmt(
        astp, loop_block, init, (*astp)->nodes[init].info.location);
    /* Block "steals" ownership of the init statement */
    (*astp)->nodes[for1].loop_for1.init = -1;

    ast_id tmp = (*astp)->nodes[init_block].block.stmt;
    (*astp)->nodes[init_block].block.stmt
        = (*astp)->nodes[loop_block].block.stmt;
    (*astp)->nodes[loop_block].block.stmt = tmp;

    /* Clean up dangling nodes */
    (*astp)->nodes[loop].loop.loop_for1 = -1;
    ast_delete_tree((*astp), for1);

    return 0;
}

static int
loop_for(
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
    ast_id       n;
    struct ast** astp = &tus[tu_id];
    struct ast*  ast = *astp;
    const char*  filename = utf8_cstr(filenames[tu_id]);
    const char*  source = sources[tu_id].text.data;

    for (n = 0; n != ast->count; ++n)
    {
        if (ast->nodes[n].info.node_type != AST_LOOP)
            continue;
        if (ast->nodes[n].loop.loop_for1 == -1)
            continue;

        if (convert_for_loop_to_primitives(astp, n, filename, source) != 0)
            return -1;
        ast = *astp;
    }

    ast_gc(ast);
#if defined(ODBCOMPILER_AST_SANITY_CHECK)
    ast_verify_connectivity(ast);
#endif
    return 0;
}

static const struct semantic_check* depends[]
    = {&semantic_unary_literal, /* Required for deducing the step direction */
       NULL};

const struct semantic_check semantic_loop_for = {loop_for, depends};
