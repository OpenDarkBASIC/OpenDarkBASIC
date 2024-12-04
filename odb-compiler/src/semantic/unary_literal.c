#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/semantic/semantic.h"

static int
process_unop(struct ast* ast, ast_id n)
{
    ast_id expr = ast->nodes[n].unop.expr;
child_changed:
    switch (ast_node_type(ast, expr))
    {
        case AST_GC: return -1;
        case AST_BLOCK: return -1;
        case AST_END: return -1;
        case AST_ARGLIST: return -1;
        case AST_PARAMLIST: return -1;
        case AST_TYPELIST: return -1;
        case AST_LOAD_PLUGIN: return -1;
        case AST_LOAD_COMMAND: return -1;
        case AST_COMMAND_NAME: return -1;
        case AST_COMMAND: return -1;
        case AST_ASSIGNMENT: return -1;
        case AST_VAR_DECL1: return -1;
        case AST_VAR_DECL2: return -1;
        case AST_VAR_READ: return -1;
        case AST_VAR_WRITE: return -1;
        case AST_UDT_DECL: return -1;
        case AST_UDT_INIT: return -1;
        case AST_UDT_READ: return -1;
        case AST_UDT_WRITE: return -1;
        case AST_PARAM: return -1;
        case AST_IDENTIFIER: return -1;
        case AST_BINOP: return -1;

        case AST_UNOP:
            if (process_unop(ast, expr) != 0)
                return -1;
            goto child_changed;

        case AST_COND: return -1;
        case AST_COND_BRANCHES: return -1;
        case AST_SELECT: return -1;
        case AST_CASELIST: return -1;
        case AST_CASE: return -1;
        case AST_LOOP1: return -1;
        case AST_LOOP2: return -1;
        case AST_LOOP_FOR1: return -1;
        case AST_LOOP_FOR2: return -1;
        case AST_LOOP_FOR3: return -1;
        case AST_LOOP_CONT: return -1;
        case AST_LOOP_EXIT: return -1;
        case AST_FUNC_POLY: return -1;
        case AST_FUNC1: return -1;
        case AST_FUNC2: return -1;
        case AST_FUNC3: return -1;
        case AST_FUNC4: return -1;
        case AST_FUNC_EXIT: return -1;
        case AST_FUNC_CALL: return -1;
        case AST_CALL_LIKE: return -1;
        case AST_CONTAINER_WRITE: return -1;

        case AST_BOOLEAN_LITERAL:
            switch (ast->nodes[n].unop.op)
            {
                case UNOP_BITWISE_NOT:
                case UNOP_LOGICAL_NOT:
                    ast->nodes[expr].boolean_literal.is_true
                        = ast->nodes[expr].boolean_literal.is_true ? 0 : 1;
                    break;
                case UNOP_NEGATE:
                    ast->nodes[expr].info.node_type = AST_INTEGER_LITERAL;
                    ast->nodes[expr].integer_literal.value
                        = -ast->nodes[expr].boolean_literal.is_true;
                    break;
            }
            break;

        case AST_BYTE_LITERAL:
            switch (ast->nodes[n].unop.op)
            {
                case UNOP_LOGICAL_NOT:
                    ast->nodes[expr].info.node_type = AST_BOOLEAN_LITERAL;
                    ast->nodes[expr].boolean_literal.is_true
                        = ast->nodes[expr].byte_literal.value ? 0 : 1;
                    break;
                case UNOP_NEGATE:
                    ast->nodes[expr].info.node_type = AST_INTEGER_LITERAL;
                    ast->nodes[expr].integer_literal.value
                        = -ast->nodes[expr].byte_literal.value;
                    break;
                case UNOP_BITWISE_NOT:
                    ast->nodes[expr].byte_literal.value
                        = ~ast->nodes[expr].byte_literal.value;
                    break;
            }
            break;

        case AST_WORD_LITERAL:
            switch (ast->nodes[n].unop.op)
            {
                case UNOP_LOGICAL_NOT:
                    ast->nodes[expr].info.node_type = AST_BOOLEAN_LITERAL;
                    ast->nodes[expr].boolean_literal.is_true
                        = ast->nodes[expr].word_literal.value ? 0 : 1;
                    break;
                case UNOP_NEGATE:
                    ast->nodes[expr].info.node_type = AST_INTEGER_LITERAL;
                    ast->nodes[expr].integer_literal.value
                        = -ast->nodes[expr].word_literal.value;
                    break;
                case UNOP_BITWISE_NOT:
                    ast->nodes[expr].word_literal.value
                        = ~ast->nodes[expr].word_literal.value;
                    break;
            }
            break;

        case AST_DWORD_LITERAL:
            switch (ast->nodes[n].unop.op)
            {
                case UNOP_LOGICAL_NOT:
                    ast->nodes[expr].info.node_type = AST_BOOLEAN_LITERAL;
                    ast->nodes[expr].boolean_literal.is_true
                        = ast->nodes[expr].dword_literal.value ? 0 : 1;
                    break;
                case UNOP_NEGATE:
                    ast->nodes[expr].info.node_type
                        = AST_DOUBLE_INTEGER_LITERAL;
                    ast->nodes[expr].double_integer_literal.value
                        = -(int64_t)ast->nodes[expr].dword_literal.value;
                    break;
                case UNOP_BITWISE_NOT:
                    ast->nodes[expr].dword_literal.value
                        = ~ast->nodes[expr].dword_literal.value;
                    break;
            }
            break;

        case AST_INTEGER_LITERAL:
            switch (ast->nodes[n].unop.op)
            {
                case UNOP_LOGICAL_NOT:
                    ast->nodes[expr].info.node_type = AST_BOOLEAN_LITERAL;
                    ast->nodes[expr].boolean_literal.is_true
                        = ast->nodes[expr].integer_literal.value ? 0 : 1;
                    break;
                case UNOP_NEGATE:
                    ast->nodes[expr].integer_literal.value
                        = -ast->nodes[expr].integer_literal.value;
                    break;
                case UNOP_BITWISE_NOT:
                    ast->nodes[expr].integer_literal.value
                        = ~ast->nodes[expr].integer_literal.value;
                    break;
            }
            break;

        case AST_DOUBLE_INTEGER_LITERAL:
            switch (ast->nodes[n].unop.op)
            {
                case UNOP_LOGICAL_NOT:
                    ast->nodes[expr].info.node_type = AST_BOOLEAN_LITERAL;
                    ast->nodes[expr].boolean_literal.is_true
                        = ast->nodes[expr].double_integer_literal.value ? 0 : 1;
                    break;
                case UNOP_NEGATE:
                    ast->nodes[expr].double_integer_literal.value
                        = -ast->nodes[expr].double_integer_literal.value;
                    break;
                case UNOP_BITWISE_NOT:
                    ast->nodes[expr].double_integer_literal.value
                        = ~ast->nodes[expr].double_integer_literal.value;
                    break;
            }
            break;

        case AST_FLOAT_LITERAL:
            switch (ast->nodes[n].unop.op)
            {
                case UNOP_LOGICAL_NOT:
                    ast->nodes[expr].info.node_type = AST_BOOLEAN_LITERAL;
                    ast->nodes[expr].boolean_literal.is_true
                        = ast->nodes[expr].float_literal.value ? 0 : 1;
                    break;
                case UNOP_NEGATE:
                    ast->nodes[expr].float_literal.value
                        = -ast->nodes[expr].float_literal.value;
                    break;
                case UNOP_BITWISE_NOT: return -1;
            }
            break;

        case AST_DOUBLE_LITERAL:
            switch (ast->nodes[n].unop.op)
            {
                case UNOP_LOGICAL_NOT:
                    ast->nodes[expr].info.node_type = AST_BOOLEAN_LITERAL;
                    ast->nodes[expr].boolean_literal.is_true
                        = ast->nodes[expr].double_literal.value ? 0 : 1;
                    break;
                case UNOP_NEGATE:
                    ast->nodes[expr].double_literal.value
                        = -ast->nodes[expr].double_literal.value;
                    break;
                case UNOP_BITWISE_NOT: return -1;
            }
            break;

        case AST_STRING_LITERAL: return -1;
        case AST_CAST: return -1;
        case AST_AS_TYPE: return -1;
        case AST_AS_EXPR: return -1;
        case AST_AS_UDT: return -1;
        case AST_AS_AUTO: return -1;
    }

    ast->nodes[expr].info.location
        = utf8_span_union(ast_loc(ast, n), ast_loc(ast, expr));
    memcpy(&ast->nodes[n], &ast->nodes[expr], sizeof(ast->nodes[expr]));
    ast_delete_node(ast, expr);
    return 0;
}

static int
unary_literal(
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
    ast_id      n;
    struct ast* ast = tus[tu_id];

    for (n = 0; n != ast_count(ast); ++n)
    {
        if (ast_node_type(ast, n) != AST_UNOP)
            continue;

        process_unop(ast, n);
    }

    return 0;
}

static const struct semantic_check* depends[] = {NULL};

const struct semantic_check semantic_unary_literal
    = {unary_literal, depends, "unary_literal"};
