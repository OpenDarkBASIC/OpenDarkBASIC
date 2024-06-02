#include "odb-compiler/ast/ast.h"
#include "odb-compiler/sdk/type.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-sdk/log.h"

enum type
type_check_and_cast_binop_arithmetic(
    struct ast*      ast,
    ast_id           op,
    const char*      source_filename,
    struct db_source source);

enum type
type_check_and_cast_binop_pow(
    struct ast*      ast,
    ast_id           op,
    const char*      source_filename,
    struct db_source source);

enum type
type_check_and_cast_casts(
    struct ast*      ast,
    ast_id           cast,
    const char*      source_filename,
    struct db_source source);

static enum type
resolve_expression(
    struct ast*            ast,
    ast_id                 n,
    const struct cmd_list* cmds,
    const char*            source_filename,
    struct db_source       source)
{
    if (ast->nodes[n].info.type_info != TYPE_VOID)
        return ast->nodes[n].info.type_info;

    switch (ast->nodes[n].info.node_type)
    {
        /* Nodes that don't return a value should be marked as TYPE_VOID */
        case AST_BLOCK:
        case AST_ARGLIST:
        case AST_CONST_DECL:
        case AST_ASSIGNMENT: return TYPE_VOID;

        case AST_COMMAND:
            return ast->nodes[n].cmd.info.type_info
                   = *vec_get(cmds->return_types, ast->nodes[n].cmd.id);

        case AST_IDENTIFIER: return ast->nodes[n].identifier.info.type_info;

        case AST_BINOP: {
            ast_id lhs = ast->nodes[n].binop.left;
            ast_id rhs = ast->nodes[n].binop.right;
            if (resolve_expression(ast, lhs, cmds, source_filename, source) < 0)
                return -1;
            if (resolve_expression(ast, rhs, cmds, source_filename, source) < 0)
                return -1;

            switch (ast->nodes[n].binop.op)
            {
                case BINOP_ADD:
                case BINOP_SUB:
                case BINOP_MUL:
                case BINOP_DIV:
                case BINOP_MOD:
                    return type_check_and_cast_binop_arithmetic(
                        ast, n, source_filename, source);

                case BINOP_POW:
                    return type_check_and_cast_binop_pow(
                        ast, n, source_filename, source);

                case BINOP_SHIFT_LEFT:
                case BINOP_SHIFT_RIGHT:
                case BINOP_BITWISE_OR:
                case BINOP_BITWISE_AND:
                case BINOP_BITWISE_XOR:
                case BINOP_BITWISE_NOT:

                case BINOP_LESS_THAN:
                case BINOP_LESS_EQUAL:
                case BINOP_GREATER_THAN:
                case BINOP_GREATER_EQUAL:
                case BINOP_EQUAL:
                case BINOP_NOT_EQUAL:
                case BINOP_LOGICAL_OR:
                case BINOP_LOGICAL_AND:
                case BINOP_LOGICAL_XOR: break;
            }
        }
        break;

        case AST_UNOP: break;

        case AST_BOOLEAN_LITERAL:
            return ast->nodes[n].boolean_literal.info.type_info = TYPE_BOOLEAN;
        case AST_BYTE_LITERAL:
            return ast->nodes[n].byte_literal.info.type_info = TYPE_BYTE;
        case AST_WORD_LITERAL:
            return ast->nodes[n].word_literal.info.type_info = TYPE_WORD;
        case AST_INTEGER_LITERAL:
            return ast->nodes[n].integer_literal.info.type_info = TYPE_INTEGER;
        case AST_DWORD_LITERAL:
            return ast->nodes[n].dword_literal.info.type_info = TYPE_DWORD;
        case AST_DOUBLE_INTEGER_LITERAL:
            return ast->nodes[n].double_integer_literal.info.type_info
                   = TYPE_LONG;
        case AST_FLOAT_LITERAL:
            return ast->nodes[n].float_literal.info.type_info = TYPE_FLOAT;
        case AST_DOUBLE_LITERAL:
            return ast->nodes[n].double_literal.info.type_info = TYPE_DOUBLE;
        case AST_STRING_LITERAL:
            return ast->nodes[n].string_literal.info.type_info = TYPE_STRING;

        case AST_CAST:
            if (resolve_expression(ast, n, cmds, source_filename, source) < 0)
                return -1;
            return type_check_and_cast_casts(ast, n, source_filename, source);
    }

    return (enum type) - 1;
}

static int
resolve_expressions(
    struct ast*            ast,
    const struct cmd_list* cmds,
    const char*            source_filename,
    struct db_source       source)
{
    ast_id n;
    for (n = 0; n != ast->node_count; ++n)
        if (resolve_expression(ast, n, cmds, source_filename, source)
            == (enum type) - 1)
        {
            return -1;
        }

    return 0;
}

static int
resolve_assignments(
    struct ast*            ast,
    const struct cmd_list* cmds,
    const char*            source_filename,
    struct db_source       source)
{
    ast_id n;
    for (n = 0; n != ast->node_count; ++n)
        switch (ast->nodes[n].info.node_type)
        {
            case AST_BLOCK:
            case AST_ARGLIST:
            case AST_CONST_DECL: break;

            /* Assignments themselves are not expressions and thus don't
             * return a value, but the rvalue is often used to determine the
             * type of the lvalue. */
            case AST_ASSIGNMENT: {
            }
            break;

            case AST_COMMAND:
            case AST_IDENTIFIER:
            case AST_BINOP:
            case AST_UNOP:
            case AST_BOOLEAN_LITERAL:
            case AST_BYTE_LITERAL:
            case AST_WORD_LITERAL:
            case AST_DWORD_LITERAL:
            case AST_INTEGER_LITERAL:
            case AST_DOUBLE_INTEGER_LITERAL:
            case AST_FLOAT_LITERAL:
            case AST_DOUBLE_LITERAL:
            case AST_STRING_LITERAL:
            case AST_CAST: break;
        }

    return 0;
}

static int
type_check_expressions(
    struct ast*               ast,
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    const char*               source_filename,
    struct db_source          source)
{
    if (resolve_expressions(ast, cmds, source_filename, source) != 0)
        return -1;

    return 0;
}

static const struct semantic_check* depends[]
    = {&semantic_expand_constant_declarations, NULL};
const struct semantic_check semantic_type_check_and_cast
    = {depends, type_check_expressions};
