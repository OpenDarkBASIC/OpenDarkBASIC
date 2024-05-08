#include "odb-compiler/ast/ast.h"
#include "odb-compiler/parser/db_parser.y.h"
#include "odb-sdk/mem.h"
#include <assert.h>

static struct source_location
source_location_from_dbltype(const struct DBLTYPE* loc)
{
    struct source_location s = {
        loc->first_line, loc->last_line, loc->first_column, loc->last_column};
    return s;
}

static int
new_node(struct ast* ast, enum ast_type type, const struct DBLTYPE* loc)
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
    ast->nodes[n].base.info.loc = source_location_from_dbltype(loc);
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
ast_block(struct ast* ast, int stmt, const struct DBLTYPE* loc)
{
    int n = new_node(ast, AST_BLOCK, loc);
    if (n < 0)
        return -1;
    ast->nodes[n].block.stmt = stmt;
    return n;
}

int
ast_block_append(
    struct ast* ast, int block, int stmt, const struct DBLTYPE* loc)
{
    int n = new_node(ast, AST_BLOCK, loc);
    if (n < 0)
        return -1;

    ODBSDK_DEBUG_ASSERT(ast->nodes[block].base.info.type == AST_BLOCK);
    while (ast->nodes[block].block.next != -1)
        block = ast->nodes[block].block.next;
    ast->nodes[block].block.next = n;
    ast->nodes[n].block.stmt = stmt;

    return n;
}

int
ast_const_decl(
    struct ast* ast, int identifier, int expr, const struct DBLTYPE* loc)
{
    int n = new_node(ast, AST_CONST_DECL, loc);
    if (n < 0)
        return -1;
    ast->nodes[n].const_decl.identifier = identifier;
    ast->nodes[n].const_decl.expr = expr;
    return n;
}

int
ast_identifier(
    struct ast*           ast,
    struct utf8_ref       name,
    enum type_annotation  annotation,
    const struct DBLTYPE* loc)
{
    int n = new_node(ast, AST_IDENTIFIER, loc);
    if (n < 0)
        return -1;
    ast->nodes[n].identifier.name = name;
    ast->nodes[n].identifier.annotation = annotation;
    return n;
}

int
ast_boolean_literal(struct ast* ast, char is_true, const struct DBLTYPE* loc)
{
    int n = new_node(ast, AST_BOOLEAN_LITERAL, loc);
    if (n < 0)
        return -1;
    ast->nodes[n].boolean_literal.is_true = is_true;
    return n;
}

int
ast_integer_literal(struct ast* ast, int value, const struct DBLTYPE* loc)
{
    int n = new_node(ast, AST_INTEGER_LITERAL, loc);
    if (n < 0)
        return -1;
    ast->nodes[n].integer_literal.value = value;
    return n;
}
