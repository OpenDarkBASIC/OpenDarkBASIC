#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/parser/db_parser.y.h"
#include "odb-sdk/config.h"
#include "odb-sdk/log.h"
#include "odb-sdk/mem.h"
#include "odb-sdk/utf8.h"
#include <assert.h>

static ast_id
new_node(struct ast* ast, enum ast_type type, struct utf8_span location)
{
    if (ast->node_count == ast->node_capacity)
    {
        size_t new_size = ast->node_capacity
                              ? sizeof(union ast_node) * ast->node_capacity * 2
                              : sizeof(union ast_node) * 128;

        union ast_node* new_nodes = mem_realloc(ast->nodes, new_size);
        if (new_nodes == NULL)
            return log_oom(new_size, "new_node()");

        ast->nodes = new_nodes;
        ast->node_capacity *= 2;
    }

    ast_id n = ast->node_count++;
    ast->nodes[n].base.info.location = location;
    ast->nodes[n].base.info.node_type = type;
    ast->nodes[n].base.info.type_info = TYPE_INVALID;
    ast->nodes[n].base.left = -1;
    ast->nodes[n].base.right = -1;

    return n;
}

void
ast_deinit(struct ast* ast)
{
    if (ast->node_count)
        mem_free(ast->nodes);
}

ast_id
ast_block(struct ast* ast, ast_id stmt, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_BLOCK, location);
    if (n < 0)
        return -1;

    ODBSDK_DEBUG_ASSERT(stmt > -1, log_parser_err("stmt: %d\n", stmt));

    ast->nodes[n].block.stmt = stmt;
    return n;
}

void
ast_block_append(struct ast* ast, ast_id block, ast_id append_block)
{
    ODBSDK_DEBUG_ASSERT(block > -1, log_parser_err("block: %d\n", block));
    ODBSDK_DEBUG_ASSERT(
        ast->nodes[block].info.node_type == AST_BLOCK,
        log_parser_err("type: %d\n", ast->nodes[block].info.node_type));
    ODBSDK_DEBUG_ASSERT(
        append_block > -1, log_parser_err("block: %d\n", block));
    ODBSDK_DEBUG_ASSERT(
        ast->nodes[append_block].info.node_type == AST_BLOCK,
        log_parser_err("type: %d\n", ast->nodes[append_block].info.node_type));

    while (ast->nodes[block].block.next != -1)
        block = ast->nodes[block].block.next;
    ast->nodes[block].block.next = append_block;
}
ast_id
ast_block_append_new(
    struct ast* ast, ast_id block, ast_id stmt, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_BLOCK, location);
    if (n < 0)
        return -1;

    ast_block_append(ast, block, n);
    ast->nodes[n].block.stmt = stmt;
    return n;
}
ast_id
ast_arglist(struct ast* ast, ast_id expr, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_ARGLIST, location);
    if (n < 0)
        return -1;

    ODBSDK_DEBUG_ASSERT(expr > -1, log_parser_err("expr: %d\n", expr));

    ast->nodes[n].arglist.expr = expr;
    return n;
}

ast_id
ast_arglist_append(
    struct ast* ast, ast_id arglist, ast_id expr, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_ARGLIST, location);
    if (n < 0)
        return -1;

    ODBSDK_DEBUG_ASSERT(arglist > -1, log_parser_err("arglist: %d\n", arglist));
    ODBSDK_DEBUG_ASSERT(
        ast->nodes[arglist].info.node_type == AST_ARGLIST,
        log_parser_err("type: %d\n", ast->nodes[arglist].info.node_type));

    while (ast->nodes[arglist].arglist.next != -1)
        arglist = ast->nodes[arglist].arglist.next;
    ast->nodes[arglist].arglist.next = n;
    ast->nodes[n].arglist.expr = expr;

    return n;
}

ast_id
ast_const_decl(
    struct ast* ast, ast_id identifier, ast_id expr, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_CONST_DECL, location);
    if (n < 0)
        return -1;

    ODBSDK_DEBUG_ASSERT(
        identifier > -1, log_parser_err("identifier: %d\n", identifier));
    ODBSDK_DEBUG_ASSERT(expr > -1, log_parser_err("expr: %d\n", expr));

    ast->nodes[n].const_decl.identifier = identifier;
    ast->nodes[n].const_decl.expr = expr;
    return n;
}

ast_id
ast_command(
    struct ast* ast, cmd_id cmd_id, ast_id arglist, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_COMMAND, location);
    if (n < 0)
        return -1;

    ODBSDK_DEBUG_ASSERT(cmd_id > -1, log_parser_err("cmd_id: %d\n", cmd_id));
    ODBSDK_DEBUG_ASSERT(
        arglist == -1 || ast->nodes[arglist].info.node_type == AST_ARGLIST,
        log_parser_err("type: %d\n", ast->nodes[arglist].info.node_type));

    ast->nodes[n].cmd.id = cmd_id;
    ast->nodes[n].cmd.arglist = arglist;
    return n;
}

ast_id
ast_assign_var(
    struct ast*      ast,
    ast_id           identifier,
    ast_id           expr,
    struct utf8_span op_location,
    struct utf8_span location)
{
    ast_id n = new_node(ast, AST_ASSIGNMENT, location);
    if (n < 0)
        return -1;

    ODBSDK_DEBUG_ASSERT(
        identifier > -1, log_parser_err("identifier: %d\n", identifier));
    ODBSDK_DEBUG_ASSERT(expr > -1, log_parser_err("expr: %d\n", expr));
    ODBSDK_DEBUG_ASSERT(
        ast->nodes[identifier].info.node_type == AST_IDENTIFIER,
        log_parser_err("type: %d\n", ast->nodes[identifier].info.node_type));

    ast->nodes[n].assignment.lvalue = identifier;
    ast->nodes[n].assignment.expr = expr;
    ast->nodes[n].assignment.op_location = op_location;

    return n;
}

ast_id
ast_identifier(
    struct ast*          ast,
    struct utf8_span     name,
    enum type_annotation annotation,
    struct utf8_span     location)
{
    ast_id n = new_node(ast, AST_IDENTIFIER, location);
    if (n < 0)
        return -1;
    ast->nodes[n].identifier.name = name;
    ast->nodes[n].identifier.annotation = annotation;
    return n;
}

ast_id
ast_binop(
    struct ast*      ast,
    enum binop_type  op,
    ast_id           left,
    ast_id           right,
    struct utf8_span op_location,
    struct utf8_span location)
{
    ast_id n = new_node(ast, AST_BINOP, location);
    if (n < 0)
        return -1;

    ODBSDK_DEBUG_ASSERT(left > -1, log_parser_err("left: %d\n", left));
    ODBSDK_DEBUG_ASSERT(right > -1, log_parser_err("right: %d\n", right));

    ast->nodes[n].binop.left = left;
    ast->nodes[n].binop.right = right;
    ast->nodes[n].binop.op_location = op_location;
    ast->nodes[n].binop.op = op;
    return n;
}

ast_id
ast_unop(
    struct ast* ast, enum unop_type op, ast_id expr, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_UNOP, location);
    if (n < 0)
        return -1;

    ODBSDK_DEBUG_ASSERT(expr > -1, log_parser_err("expr: %d\n", expr));

    ast->nodes[n].unop.expr = expr;
    ast->nodes[n].unop.op = op;
    return n;
}

ast_id
ast_inc_step(
    struct ast* ast, ast_id var, ast_id expr, struct utf8_span location)
{
    ast_id add = ast_binop(ast, BINOP_ADD, var, expr, location, location);
    var = ast_dup_lvalue(ast, var);
    return ast_assign_var(ast, var, add, location, location);
}

ast_id
ast_inc(struct ast* ast, ast_id var, struct utf8_span location)
{
    ast_id expr = ast_byte_literal(ast, 1, location);
    return ast_inc_step(ast, var, expr, location);
}

ast_id
ast_dec_step(
    struct ast* ast, ast_id var, ast_id expr, struct utf8_span location)
{
    ast_id add = ast_binop(ast, BINOP_SUB, var, expr, location, location);
    var = ast_dup_lvalue(ast, var);
    return ast_assign_var(ast, var, add, location, location);
}

ast_id
ast_dec(struct ast* ast, ast_id var, struct utf8_span location)
{
    ast_id expr = ast_byte_literal(ast, 1, location);
    return ast_dec_step(ast, var, expr, location);
}

ast_id
ast_cond(
    struct ast* ast, ast_id expr, ast_id cond_branch, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_COND, location);
    if (n < 0)
        return -1;

    ODBSDK_DEBUG_ASSERT(expr > -1, log_parser_err("expr: %d\n", expr));
    ODBSDK_DEBUG_ASSERT(
        cond_branch > -1, log_parser_err("cond_branch: %d\n", cond_branch));
    ODBSDK_DEBUG_ASSERT(
        ast->nodes[cond_branch].info.node_type == AST_COND_BRANCH,
        log_parser_err("type: %d\n", ast->nodes[cond_branch].info.node_type));

    ast->nodes[n].cond.expr = expr;
    ast->nodes[n].cond.cond_branch = cond_branch;
    return n;
}

ast_id
ast_cond_branch(
    struct ast* ast, ast_id yes, ast_id no, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_COND_BRANCH, location);
    if (n < 0)
        return -1;

    ODBSDK_DEBUG_ASSERT(
        yes == -1 || ast->nodes[yes].info.node_type == AST_BLOCK,
        log_parser_err("type: %d\n", ast->nodes[yes].info.node_type));
    ODBSDK_DEBUG_ASSERT(
        no == -1 || ast->nodes[no].info.node_type == AST_BLOCK,
        log_parser_err("type: %d\n", ast->nodes[no].info.node_type));

    ast->nodes[n].cond_branch.yes = yes;
    ast->nodes[n].cond_branch.no = no;

    return n;
}

ast_id
ast_loop(
    struct ast*      ast,
    ast_id           body,
    struct utf8_span name,
    struct utf8_span implicit_name,
    struct utf8_span location)
{
    ast_id n = new_node(ast, AST_LOOP, location);
    if (n < 0)
        return -1;
    ast->nodes[n].loop.body = body;
    ast->nodes[n].loop.name = name;
    ast->nodes[n].loop.implicit_name = implicit_name;
    return n;
}

ast_id
ast_loop_while(
    struct ast*      ast,
    ast_id           body,
    ast_id           expr,
    struct utf8_span name,
    struct utf8_span location)
{
    ast_id exit = ast_loop_exit(ast, empty_utf8_span(), location);
    ast_id exit_block = ast_block(ast, exit, location);
    ast_id cond_branch = ast_cond_branch(ast, -1, exit_block, location);
    ast_id cond = ast_cond(ast, expr, cond_branch, location);
    ast_id block = ast_block(ast, cond, location);
    if (body > -1)
        ast_block_append(ast, block, body);
    return ast_loop(ast, block, name, empty_utf8_span(), location);
}

ast_id
ast_loop_until(
    struct ast*      ast,
    ast_id           body,
    ast_id           expr,
    struct utf8_span name,
    struct utf8_span location)
{
    ast_id exit = ast_loop_exit(ast, empty_utf8_span(), location);
    ast_id exit_block = ast_block(ast, exit, location);
    ast_id cond_branch = ast_cond_branch(ast, exit_block, -1, location);
    ast_id cond = ast_cond(ast, expr, cond_branch, location);
    if (body > -1)
        ast_block_append_new(ast, body, cond, location);
    else
        body = cond;
    return ast_loop(ast, body, name, empty_utf8_span(), location);
}

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

    if (step == -1 && (begin_sign == SIGN_UNKNOWN || end_sign == SIGN_UNKNOWN))
    {
        int              gutter;
        struct utf8_span loc = utf8_span_union(
            ast->nodes[begin].info.location, ast->nodes[end].info.location);
        const struct log_highlight hl_step_forwards[]
            = {{" STEP 1", "", {loc.off + loc.len, 7}, LOG_INSERT, 0},
               LOG_HIGHLIGHT_SENTINAL};
        const struct log_highlight hl_step_backwards[]
            = {{" STEP -1", "", {loc.off + loc.len, 8}, LOG_INSERT, 0},
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

    // if (step > -1 && )
    //{
    //     enum eval_sign_result step_sign = eval_constant_expr_sign(ast, step);
    // }

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

ast_id
ast_loop_for(
    struct ast*      ast,
    ast_id           body,
    ast_id           init,
    ast_id           end,
    ast_id           step,
    ast_id           next,
    struct utf8_span name,
    struct utf8_span location,
    const char*      source_filename,
    struct db_source source)
{
    ODBSDK_DEBUG_ASSERT(init > -1, log_parser_err("init: %d\n", init));
    ODBSDK_DEBUG_ASSERT(
        ast->nodes[init].info.node_type == AST_ASSIGNMENT,
        log_parser_err("type: %d\n", ast->nodes[init].info.node_type));
    ast_id loop_var = ast->nodes[init].assignment.lvalue;
    ast_id begin = ast->nodes[init].assignment.expr;
    ODBSDK_DEBUG_ASSERT(end > -1, log_parser_err("init: %d\n", end));

    ODBSDK_DEBUG_ASSERT(
        ast->nodes[loop_var].info.node_type == AST_IDENTIFIER,
        log_parser_err("type: %d\n", ast->nodes[loop_var].info.node_type));
    struct utf8_span implicit_name = ast->nodes[loop_var].identifier.name;

    ast_id exit_stmt = ast_loop_exit_stmt(
        ast, begin, end, step, loop_var, location, source_filename, source);
    if (exit_stmt < 0)
        return -1;

    ast_id inc_var = ast_dup_lvalue(ast, loop_var);
    ast_id inc_stmt
        = step > -1
              ? ast_inc_step(ast, inc_var, step, ast->nodes[step].info.location)
              : ast_inc(ast, inc_var, ast->nodes[inc_var].info.location);

    ast_id block = ast_block(ast, exit_stmt, location);
    if (body > -1)
        ast_block_append(ast, block, body);
    ast_block_append_new(
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

    ast_id loop_stmt = ast_loop(ast, block, name, implicit_name, location);
    ast_id init_block = ast_block(ast, init, location);
    ast_block_append_new(ast, init_block, loop_stmt, location);
    return init_block;
}

ast_id
ast_loop_exit(struct ast* ast, struct utf8_span name, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_LOOP_EXIT, location);
    if (n < -1)
        return -1;
    ast->nodes[n].exit.name = name;
    return n;
}

ast_id
ast_label(struct ast* ast, struct utf8_span name, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_LABEL, location);
    if (n < 0)
        return -1;
    ast->nodes[n].label.name = name;
    return n;
}

ast_id
ast_boolean_literal(struct ast* ast, char is_true, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_BOOLEAN_LITERAL, location);
    if (n < 0)
        return -1;
    ast->nodes[n].boolean_literal.is_true = is_true;
    return n;
}

ast_id
ast_byte_literal(struct ast* ast, uint8_t value, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_BYTE_LITERAL, location);
    if (n < 0)
        return -1;
    ast->nodes[n].byte_literal.value = value;
    return n;
}
ast_id
ast_word_literal(struct ast* ast, uint16_t value, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_WORD_LITERAL, location);
    if (n < 0)
        return -1;
    ast->nodes[n].word_literal.value = value;
    return n;
}
ast_id
ast_integer_literal(struct ast* ast, int32_t value, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_INTEGER_LITERAL, location);
    if (n < 0)
        return -1;
    ast->nodes[n].integer_literal.value = value;
    return n;
}
ast_id
ast_dword_literal(struct ast* ast, uint32_t value, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_DWORD_LITERAL, location);
    if (n < 0)
        return -1;
    ast->nodes[n].dword_literal.value = value;
    return n;
}
ast_id
ast_double_integer_literal(
    struct ast* ast, int64_t value, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_DOUBLE_INTEGER_LITERAL, location);
    if (n < 0)
        return -1;
    ast->nodes[n].double_integer_literal.value = value;
    return n;
}
ast_id
ast_integer_like_literal(
    struct ast* ast, int64_t value, struct utf8_span location)
{
    if (value >= 0)
    {
        if (value > UINT32_MAX)
            return ast_double_integer_literal(ast, value, location);
        if (value > INT32_MAX)
            return ast_dword_literal(ast, (uint32_t)value, location);
        if (value > UINT16_MAX)
            return ast_integer_literal(ast, (int32_t)value, location);
        if (value > UINT8_MAX)
            return ast_word_literal(ast, (uint16_t)value, location);
        return ast_byte_literal(ast, (uint8_t)value, location);
    }

    if (value < INT32_MIN)
        return ast_double_integer_literal(ast, value, location);
    return ast_integer_literal(ast, (int32_t)value, location);
}

ast_id
ast_float_literal(struct ast* ast, float value, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_FLOAT_LITERAL, location);
    if (n < 0)
        return -1;
    ast->nodes[n].float_literal.value = value;
    return n;
}
ast_id
ast_double_literal(struct ast* ast, double value, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_DOUBLE_LITERAL, location);
    if (n < 0)
        return -1;
    ast->nodes[n].double_literal.value = value;
    return n;
}

ast_id
ast_string_literal(
    struct ast* ast, struct utf8_span str, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_STRING_LITERAL, location);
    if (n < 0)
        return -1;
    ast->nodes[n].string_literal.str = str;
    return n;
}

ast_id
ast_cast(
    struct ast*      ast,
    ast_id           expr,
    enum type        target_type,
    struct utf8_span location)
{
    ast_id n = new_node(ast, AST_CAST, location);
    if (n < 0)
        return -1;

    ODBSDK_DEBUG_ASSERT(expr > -1, log_parser_err("expr: %d\n", expr));
    ast->nodes[n].cast.expr = expr;
    ast->nodes[n].info.type_info = target_type;
    return n;
}
