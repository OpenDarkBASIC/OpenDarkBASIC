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
        case AST_GC:
        case AST_BLOCK:
        case AST_END:
        case AST_ARGLIST:
        case AST_PARAMLIST:
        case AST_COMMAND:
        case AST_ASSIGNMENT:
        case AST_IDENTIFIER:
        case AST_BINOP: return -1;

        case AST_UNOP:
            if (process_unop(ast, expr) != 0)
                return -1;
            goto child_changed;

        case AST_COND: return -1;
        case AST_COND_BRANCH: return -1;
        case AST_LOOP: return -1;
        case AST_LOOP_BODY: return -1;
        case AST_LOOP_FOR1: return -1;
        case AST_LOOP_FOR2: return -1;
        case AST_LOOP_FOR3: return -1;
        case AST_LOOP_CONT: return -1;
        case AST_LOOP_EXIT: return -1;
        case AST_FUNC_TEMPLATE: return -1;
        case AST_FUNC: return -1;
        case AST_FUNC_DECL: return -1;
        case AST_FUNC_DEF: return -1;
        case AST_FUNC_EXIT: return -1;
        case AST_FUNC_OR_CONTAINER_REF: return -1;
        case AST_FUNC_CALL: return -1;

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
        case AST_SCOPE: return -1;
    }

    ast->nodes[expr].info.location = utf8_span_union(
        ast->nodes[n].info.location, ast->nodes[expr].info.location);
    memcpy(&ast->nodes[n], &ast->nodes[expr], sizeof(ast->nodes[expr]));
    ast_delete_node(ast, expr);
    return 0;
}

static int
unary_literal(
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

const struct semantic_check semantic_unary_literal = {unary_literal, depends};
