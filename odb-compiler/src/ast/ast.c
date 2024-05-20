#include "odb-compiler/ast/ast.h"
#include "odb-compiler/parser/db_parser.y.h"
#include "odb-sdk/config.h"
#include "odb-sdk/mem.h"
#include <assert.h>

static int
new_node(struct ast* ast, enum ast_type type, struct utf8_span location)
{
    if (ast->node_count == ast->node_capacity)
    {
        size_t new_size = ast->node_capacity
                              ? sizeof(union ast_node) * ast->node_capacity * 2
                              : sizeof(union ast_node) * 128;

        union ast_node* new_nodes = mem_realloc(ast->nodes, new_size);
        if (new_nodes == NULL)
            return mem_report_oom(new_size, "new_node()");

        ast->nodes = new_nodes;
        ast->node_capacity *= 2;
    }

    int n = ast->node_count++;
    ast->nodes[n].base.info.location = location;
    ast->nodes[n].base.info.type = type;
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

int
ast_block(struct ast* ast, int stmt, struct utf8_span location)
{
    int n = new_node(ast, AST_BLOCK, location);
    if (n < 0)
        return -1;
    ast->nodes[n].block.stmt = stmt;
    return n;
}

int
ast_block_append(
    struct ast* ast, int block, int stmt, struct utf8_span location)
{
    int n = new_node(ast, AST_BLOCK, location);
    if (n < 0)
        return -1;

    ODBSDK_DEBUG_ASSERT(block > -1);
    ODBSDK_DEBUG_ASSERT(ast->nodes[block].base.info.type == AST_BLOCK);
    while (ast->nodes[block].block.next != -1)
        block = ast->nodes[block].block.next;
    ast->nodes[block].block.next = n;
    ast->nodes[n].block.stmt = stmt;

    return n;
}
int
ast_arglist(struct ast* ast, int expr, struct utf8_span location)
{
    int n = new_node(ast, AST_ARGLIST, location);
    if (n < 0)
        return -1;
    ast->nodes[n].arglist.expr = expr;
    return n;
}

int
ast_arglist_append(
    struct ast* ast, int arglist, int expr, struct utf8_span location)
{
    int n = new_node(ast, AST_ARGLIST, location);
    if (n < 0)
        return -1;

    ODBSDK_DEBUG_ASSERT(arglist > -1);
    ODBSDK_DEBUG_ASSERT(ast->nodes[arglist].base.info.type == AST_ARGLIST);
    while (ast->nodes[arglist].arglist.next != -1)
        arglist = ast->nodes[arglist].arglist.next;
    ast->nodes[arglist].arglist.next = n;
    ast->nodes[n].arglist.expr = expr;

    return n;
}

int
ast_const_decl(
    struct ast* ast, int identifier, int expr, struct utf8_span location)
{
    int n = new_node(ast, AST_CONST_DECL, location);
    if (n < 0)
        return -1;
    ast->nodes[n].const_decl.identifier = identifier;
    ast->nodes[n].const_decl.expr = expr;
    return n;
}

int
ast_command(
    struct ast* ast, cmd_id cmd_idx, int arglist, struct utf8_span location)
{
    int n = new_node(ast, AST_COMMAND, location);
    if (n < 0)
        return -1;
    ast->nodes[n].cmd.id = cmd_idx;
    ast->nodes[n].cmd.arglist = arglist;
    return n;
}

int
ast_assign_var(
    struct ast* ast, int var_ref, int expr, struct utf8_span location)
{
    int n = new_node(ast, AST_ASSIGN, location);
    if (n < 0)
        return -1;
    ast->nodes[n].assign_var.var_ref = var_ref;
    ast->nodes[n].assign_var.expr = expr;
    return n;
}

int
ast_identifier(
    struct ast*          ast,
    struct utf8_span     name,
    enum type_annotation annotation,
    struct utf8_span     location)
{
    int n = new_node(ast, AST_IDENTIFIER, location);
    if (n < 0)
        return -1;
    ast->nodes[n].identifier.name = name;
    ast->nodes[n].identifier.annotation = annotation;
    return n;
}

int
ast_boolean_literal(struct ast* ast, char is_true, struct utf8_span location)
{
    int n = new_node(ast, AST_BOOLEAN_LITERAL, location);
    if (n < 0)
        return -1;
    ast->nodes[n].boolean_literal.is_true = is_true;
    return n;
}

static int
ast_byte_literal(struct ast* ast, uint8_t value, struct utf8_span location)
{
    int n = new_node(ast, AST_BYTE_LITERAL, location);
    if (n < 0)
        return -1;
    ast->nodes[n].byte_literal.value = value;
    return n;
}
static int
ast_word_literal(struct ast* ast, uint16_t value, struct utf8_span location)
{
    int n = new_node(ast, AST_WORD_LITERAL, location);
    if (n < 0)
        return -1;
    ast->nodes[n].word_literal.value = value;
    return n;
}
static int
ast_integer_literal(struct ast* ast, int32_t value, struct utf8_span location)
{
    int n = new_node(ast, AST_INTEGER_LITERAL, location);
    if (n < 0)
        return -1;
    ast->nodes[n].integer_literal.value = value;
    return n;
}
static int
ast_dword_literal(struct ast* ast, uint32_t value, struct utf8_span location)
{
    int n = new_node(ast, AST_DWORD_LITERAL, location);
    if (n < 0)
        return -1;
    ast->nodes[n].dword_literal.value = value;
    return n;
}
static int
ast_double_integer_literal(
    struct ast* ast, uint64_t value, struct utf8_span location)
{
    int n = new_node(ast, AST_DOUBLE_INTEGER_LITERAL, location);
    if (n < 0)
        return -1;
    ast->nodes[n].double_integer_literal.value = value;
    return n;
}
int
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

int
ast_float_literal(struct ast* ast, float value, struct utf8_span location)
{
    int n = new_node(ast, AST_FLOAT_LITERAL, location);
    if (n < 0)
        return -1;
    ast->nodes[n].float_literal.value = value;
    return n;
}
int
ast_double_literal(struct ast* ast, double value, struct utf8_span location)
{
    int n = new_node(ast, AST_DOUBLE_LITERAL, location);
    if (n < 0)
        return -1;
    ast->nodes[n].double_literal.value = value;
    return n;
}

int
ast_string_literal(
    struct ast* ast, struct utf8_span str, struct utf8_span location)
{
    int n = new_node(ast, AST_STRING_LITERAL, location);
    if (n < 0)
        return -1;
    ast->nodes[n].string_literal.str = str;
    return n;
}
