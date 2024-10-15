#include "odb-compiler/ast/ast.h"
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
        case AST_LOOP_FOR: break;
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
handle_next_stmt(
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
    ast_delete_tree(ast, next);
}

static ast_id
primitives_from_for_loop(
    struct ast** astp,
    ast_id       loop,
    const char*  source_filename,
    const char*  source_text)
{
    ast_id           loop_for;
    ast_id           init, loop_var, begin, end, step, next, body;
    struct utf8_span loop_loc;

    loop_for = (*astp)->nodes[loop].loop.loop_for;
    ODBUTIL_DEBUG_ASSERT(
        loop_for > -1, log_semantic_err("loop_for: %d\n", loop_for));
    ODBUTIL_DEBUG_ASSERT(
        (*astp)->nodes[loop].loop.post_body == -1,
        log_semantic_err(
            "post_body: %d\n", (*astp)->nodes[loop].loop.post_body));

    loop_loc = (*astp)->nodes[loop].info.location;
    body = (*astp)->nodes[loop].loop.body;
    init = (*astp)->nodes[loop_for].loop_for.init;
    end = (*astp)->nodes[loop_for].loop_for.end;
    step = (*astp)->nodes[loop_for].loop_for.step;
    next = (*astp)->nodes[loop_for].loop_for.next;
    ODBUTIL_DEBUG_ASSERT(init > -1, log_semantic_err("init: %d\n", init));
    ODBUTIL_DEBUG_ASSERT(end > -1, log_semantic_err("end: %d\n", end));

    ODBUTIL_DEBUG_ASSERT(
        (*astp)->nodes[init].info.node_type == AST_ASSIGNMENT,
        log_semantic_err("type: %d\n", (*astp)->nodes[init].info.node_type));
    loop_var = (*astp)->nodes[init].assignment.lvalue;
    begin = (*astp)->nodes[init].assignment.expr;

    if (next > -1)
        handle_next_stmt((*astp), next, loop_var, source_filename, source_text);

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

    ast_id inc_var = ast_dup_lvalue(astp, loop_var);
    ast_id inc_stmt
        = step > -1
              ? ast_inc_step(
                    astp, inc_var, step, (*astp)->nodes[step].info.location)
              : ast_inc(astp, inc_var, (*astp)->nodes[inc_var].info.location);

    ast_id new_body = ast_block(astp, exit_stmt, loop_loc);
    if (body > -1)
        ast_block_append((*astp), new_body, body);

    (*astp)->nodes[loop].loop.body = new_body;
    (*astp)->nodes[loop].loop.post_body
        = ast_block(astp, inc_stmt, (*astp)->nodes[inc_stmt].info.location);
    (*astp)->nodes[loop].loop.loop_for = -1;

    ast_id init_block = ast_find_parent((*astp), loop);
    ODBUTIL_DEBUG_ASSERT(
        init_block > -1, log_semantic_err("init_block: %d\n", init_block));
    ODBUTIL_DEBUG_ASSERT(
        (*astp)->nodes[init_block].info.node_type == AST_BLOCK,
        log_semantic_err(
            "init_block: %d\n", (*astp)->nodes[init_block].info.node_type));

    ast_id loop_block = loop_for;
    (*astp)->nodes[loop_block].info.node_type = AST_BLOCK;
    (*astp)->nodes[loop_block].info.location = loop_loc;
    (*astp)->nodes[loop_block].block.stmt = loop;
    (*astp)->nodes[loop_block].block.next
        = (*astp)->nodes[init_block].block.next;

    (*astp)->nodes[init_block].info.location = loop_loc;
    (*astp)->nodes[init_block].block.next = loop_block;
    (*astp)->nodes[init_block].block.stmt = init;

    return 0;
}

static int
translate_loop_for(
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
        if (ast->nodes[n].loop.loop_for == -1)
            continue;

        if (primitives_from_for_loop(astp, n, filename, source) != 0)
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

const struct semantic_check semantic_loop_for = {translate_loop_for, depends};
