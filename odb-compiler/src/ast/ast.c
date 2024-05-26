#include "odb-compiler/ast/ast.h"
#include "odb-compiler/parser/db_parser.y.h"
#include "odb-sdk/config.h"
#include "odb-sdk/mem.h"
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
    ast->nodes[n].base.info.type_info = TYPE_VOID;
    ast->nodes[n].base.parent = -1;
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
    ast->nodes[n].block.stmt = stmt;
    return n;
}

ast_id
ast_block_append(
    struct ast* ast, ast_id block, ast_id stmt, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_BLOCK, location);
    if (n < 0)
        return -1;

    ODBSDK_DEBUG_ASSERT(block > -1);
    ODBSDK_DEBUG_ASSERT(ast->nodes[block].base.info.node_type == AST_BLOCK);
    while (ast->nodes[block].block.next != -1)
        block = ast->nodes[block].block.next;
    ast->nodes[block].block.next = n;
    ast->nodes[n].block.stmt = stmt;

    return n;
}
ast_id
ast_arglist(struct ast* ast, ast_id expr, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_ARGLIST, location);
    if (n < 0)
        return -1;
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

    ODBSDK_DEBUG_ASSERT(arglist > -1);
    ODBSDK_DEBUG_ASSERT(ast->nodes[arglist].base.info.node_type == AST_ARGLIST);
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
    ast->nodes[n].const_decl.identifier = identifier;
    ast->nodes[n].const_decl.expr = expr;
    return n;
}

ast_id
ast_command(
    struct ast* ast, cmd_id cmd_idx, ast_id arglist, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_COMMAND, location);
    if (n < 0)
        return -1;
    ast->nodes[n].cmd.id = cmd_idx;
    ast->nodes[n].cmd.arglist = arglist;
    return n;
}

ast_id
ast_assign_var(
    struct ast* ast, ast_id identifier, ast_id expr, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_ASSIGNMENT, location);
    if (n < 0)
        return -1;
    ast->nodes[n].assignment.lvalue = identifier;
    ast->nodes[n].assignment.expr = expr;
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
    struct utf8_span location)
{
    ast_id n = new_node(ast, AST_BINOP, location);
    if (n < 0)
        return -1;
    ast->nodes[n].binop.left = left;
    ast->nodes[n].binop.right = right;
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
    ast->nodes[n].unop.expr = expr;
    ast->nodes[n].unop.op = op;
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

static ast_id
ast_byte_literal(struct ast* ast, uint8_t value, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_BYTE_LITERAL, location);
    if (n < 0)
        return -1;
    ast->nodes[n].byte_literal.value = value;
    return n;
}
static ast_id
ast_word_literal(struct ast* ast, uint16_t value, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_WORD_LITERAL, location);
    if (n < 0)
        return -1;
    ast->nodes[n].word_literal.value = value;
    return n;
}
static ast_id
ast_integer_literal(struct ast* ast, int32_t value, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_INTEGER_LITERAL, location);
    if (n < 0)
        return -1;
    ast->nodes[n].integer_literal.value = value;
    return n;
}
static ast_id
ast_dword_literal(struct ast* ast, uint32_t value, struct utf8_span location)
{
    ast_id n = new_node(ast, AST_DWORD_LITERAL, location);
    if (n < 0)
        return -1;
    ast->nodes[n].dword_literal.value = value;
    return n;
}
static ast_id
ast_double_integer_literal(
    struct ast* ast, uint64_t value, struct utf8_span location)
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
