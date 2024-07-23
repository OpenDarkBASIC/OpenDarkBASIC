#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/semantic/semantic.h"

enum eval_sign_result
{
    SIGN_UNKNOWN,
    SIGN_POSITIVE,
    SIGN_NEGATIVE,
};

static enum eval_sign_result
eval_constant_expr_sign(const struct ast* ast, ast_id n)
{
    switch (ast->nodes[n].info.node_type)
    {
        case AST_GC:
        case AST_BLOCK:
        case AST_ARGLIST:
        case AST_CONST_DECL:
        case AST_COMMAND:
        case AST_ASSIGNMENT:
        case AST_IDENTIFIER: break;

        case AST_BINOP:
        case AST_UNOP:
        case AST_COND:
        case AST_COND_BRANCH:
        case AST_LOOP:
        case AST_LOOP_FOR:
        case AST_LOOP_EXIT:
        case AST_LABEL: break;

        case AST_BOOLEAN_LITERAL:
        case AST_BYTE_LITERAL:
        case AST_WORD_LITERAL:
        case AST_DWORD_LITERAL: return SIGN_POSITIVE;

        case AST_INTEGER_LITERAL:
            return ast->nodes[n].integer_literal.value >= 0 ? SIGN_POSITIVE
                                                            : SIGN_NEGATIVE;
        case AST_DOUBLE_INTEGER_LITERAL:
            return ast->nodes[n].double_integer_literal.value >= 0
                       ? SIGN_POSITIVE
                       : SIGN_NEGATIVE;
        case AST_FLOAT_LITERAL:
            return ast->nodes[n].float_literal.value >= 0.0f ? SIGN_POSITIVE
                                                             : SIGN_NEGATIVE;
        case AST_DOUBLE_LITERAL:
            return ast->nodes[n].double_literal.value >= 0.0 ? SIGN_POSITIVE
                                                             : SIGN_NEGATIVE;

        case AST_STRING_LITERAL: break;

        case AST_CAST:
            return eval_constant_expr_sign(ast, ast->nodes[n].cast.expr);
    }

    return SIGN_UNKNOWN;
}

static ast_id
ast_loop_exit_stmt(
    struct ast*      ast,
    ast_id           begin,
    ast_id           end,
    ast_id           step,
    ast_id           loop_var,
    struct utf8_span location,
    const char*      source_filename,
    struct db_source source)
{
    enum eval_sign_result begin_sign = eval_constant_expr_sign(ast, begin);
    enum eval_sign_result end_sign = eval_constant_expr_sign(ast, end);
    enum eval_sign_result step_sign
        = step > -1 ? eval_constant_expr_sign(ast, step) : SIGN_UNKNOWN;

    if (step == -1 && (begin_sign == SIGN_UNKNOWN || end_sign == SIGN_UNKNOWN))
    {
        int              gutter;
        struct utf8_span loc = utf8_span_union(
            ast->nodes[begin].info.location, ast->nodes[end].info.location);
        utf8_idx             ins = loc.off + loc.len;
        struct log_highlight hl_step_forwards[]
            = {{" STEP 1", "", {ins, 7}, LOG_INSERT, LOG_MARKERS, 0},
               LOG_HIGHLIGHT_SENTINAL};
        struct log_highlight hl_step_backwards[]
            = {{" STEP -1", "", {ins, 8}, LOG_INSERT, LOG_MARKERS, 0},
               LOG_HIGHLIGHT_SENTINAL};
        log_flc_warn(
            source_filename,
            source.text.data,
            loc,
            "For-loop direction may be incorrect.\n");
        gutter = log_excerpt_1(source.text.data, loc, "");
        log_excerpt_help(
            gutter,
            "If no STEP is specified, it will default to 1. You can silence "
            "this warning by making the STEP explicit:\n");
        log_excerpt(source.text.data, hl_step_forwards);
        log_excerpt(source.text.data, hl_step_backwards);
    }

    if (step > -1 && step_sign == SIGN_UNKNOWN
        && (begin_sign == SIGN_UNKNOWN || end_sign == SIGN_UNKNOWN))
    {
        int              gutter;
        struct utf8_span loc1 = utf8_span_union(
            ast->nodes[begin].info.location, ast->nodes[end].info.location);
        struct utf8_span     loc2 = ast->nodes[step].info.location;
        struct log_highlight hl[]
            = {{"", "", loc1, LOG_HIGHLIGHT, LOG_MARKERS, 0},
               {"", "", loc2, LOG_HIGHLIGHT, LOG_MARKERS, 0},
               LOG_HIGHLIGHT_SENTINAL};
        log_flc_err(
            source_filename,
            source.text.data,
            loc1,
            "Unable to determine direction of for-loop.\n");
        gutter = log_excerpt(source.text.data, hl);
        log_excerpt_note(
            gutter,
            "The direction a for-loop counts must be known at compile-time, "
            "because the exit condition depends on it. You can either make the "
            "STEP value a constant, or make both the start and end values "
            "constants.\n");
        return -1;
    }

    if (begin_sign != SIGN_UNKNOWN && end_sign != SIGN_UNKNOWN
        && step_sign != SIGN_UNKNOWN)
    {
    }

    ast_id exit = ast_loop_exit(ast, empty_utf8_span(), location);
    ast_id exit_cond_block = ast_block(ast, exit, location);
    ast_id exit_cond_branch
        = ast_cond_branch(ast, exit_cond_block, -1, location);
    ast_id exit_var = ast_dup_lvalue(ast, loop_var);
    ast_id exit_expr
        = ast_binop(ast, BINOP_GREATER_THAN, exit_var, end, location, location);
    ast_id exit_stmt = ast_cond(ast, exit_expr, exit_cond_branch, location);

    return exit_stmt;
}

static ast_id
primitive_block_from_for_loop(
    struct ast*      ast,
    ast_id           loop,
    const char*      source_filename,
    struct db_source source)
{
    ast_id           loop_for;
    ast_id           init, loop_var, begin, end, step, next, body;
    struct utf8_span name, implicit_name;
    struct utf8_span loop_loc;

    loop_for = ast->nodes[loop].loop.loop_for;
    ODBSDK_DEBUG_ASSERT(
        loop_for > -1, log_parser_err("loop_for: %d\n", loop_for));

    loop_loc = ast->nodes[loop].info.location;
    body = ast->nodes[loop].loop.body;
    init = ast->nodes[loop_for].loop_for.init;
    end = ast->nodes[loop_for].loop_for.end;
    step = ast->nodes[loop_for].loop_for.step;
    next = ast->nodes[loop_for].loop_for.next;
    ODBSDK_DEBUG_ASSERT(init > -1, log_parser_err("init: %d\n", init));
    ODBSDK_DEBUG_ASSERT(end > -1, log_parser_err("end: %d\n", end));

    ODBSDK_DEBUG_ASSERT(
        ast->nodes[init].info.node_type == AST_ASSIGNMENT,
        log_parser_err("type: %d\n", ast->nodes[init].info.node_type));
    loop_var = ast->nodes[init].assignment.lvalue;
    begin = ast->nodes[init].assignment.expr;

    ODBSDK_DEBUG_ASSERT(
        ast->nodes[loop_var].info.node_type == AST_IDENTIFIER,
        log_parser_err("type: %d\n", ast->nodes[loop_var].info.node_type));
    name = ast->nodes[loop].loop.name;
    implicit_name = ast->nodes[loop_var].identifier.name;

    ast_id exit_stmt = ast_loop_exit_stmt(
        ast, begin, end, step, loop_var, loop_loc, source_filename, source);
    if (exit_stmt < 0)
        return -1;

    ast_id inc_var = ast_dup_lvalue(ast, loop_var);
    ast_id inc_stmt
        = step > -1
              ? ast_inc_step(ast, inc_var, step, ast->nodes[step].info.location)
              : ast_inc(ast, inc_var, ast->nodes[inc_var].info.location);

    ast_id block = ast_block(ast, exit_stmt, loop_loc);
    if (body > -1)
        ast_block_append(ast, block, body);
    ast_block_append_stmt(
        ast, block, inc_stmt, ast->nodes[inc_stmt].info.location);

    if (next > -1)
    {
        if (!ast_trees_equal(source, ast, loop_var, next))
        {
            int gutter;
            log_flc_warn(
                source_filename,
                source.text.data,
                ast->nodes[next].info.location,
                "Loop variable in next statement is different from the one "
                "used in the for-loop statement.\n");
            gutter = log_excerpt_1(
                source.text.data, ast->nodes[next].info.location, "");
            log_excerpt_note(gutter, "Loop variable declared here:\n");
            log_excerpt_1(
                source.text.data, ast->nodes[loop_var].info.location, "");
        }
        ast_delete_tree(ast, next);
    }

    ast_id loop_stmt = ast_loop(ast, block, name, implicit_name, loop_loc);
    ast_id init_block = loop;
    ast->nodes[init_block].info.node_type = AST_BLOCK;
    ast->nodes[init_block].block.stmt = init;
    ast->nodes[init_block].block.next = -1;
    ast_block_append_stmt(ast, init_block, loop_stmt, loop_loc);
    ast_delete_node(ast, loop_for);

    return 0;
}

static int
translate_loop_for(
    struct ast*               ast,
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    const char*               source_filename,
    struct db_source          source)
{
    ast_id n;
    for (n = 0; n != ast->node_count; ++n)
    {
        if (ast->nodes[n].info.node_type != AST_LOOP)
            continue;
        if (ast->nodes[n].loop.loop_for == -1)
            continue;

        if (primitive_block_from_for_loop(ast, n, source_filename, source) != 0)
            return -1;
    }

    ast_gc(ast);
    return 0;
}

static const struct semantic_check* depends[]
    = {&semantic_expand_constant_declarations, NULL};

const struct semantic_check semantic_loop_for = {translate_loop_for, depends};
