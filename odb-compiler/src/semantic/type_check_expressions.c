#include "odb-compiler/ast/ast.h"
#include "odb-compiler/sdk/type.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-sdk/log.h"

static enum type
resolve_expression(
    struct ast*            ast,
    ast_id                 n,
    const struct cmd_list* cmds,
    const char*            source_filename,
    struct db_source       source)
{
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
            ast_id    lhs = ast->nodes[n].binop.left;
            ast_id    rhs = ast->nodes[n].binop.right;
            enum type left_type
                = resolve_expression(ast, lhs, cmds, source_filename, source);
            enum type right_type
                = resolve_expression(ast, rhs, cmds, source_filename, source);

            enum type_promotion_result left_to_right
                = type_promote(left_type, right_type);
            enum type_promotion_result right_to_left
                = type_promote(right_type, left_type);

            /* Prefer TP_ALLOW over TP_STRANGE */
            if (left_to_right == TP_ALLOW)
                return ast->nodes[n].info.type_info = right_type;
            if (right_to_left == TP_ALLOW)
                return ast->nodes[n].info.type_info = left_type;

            if (left_to_right == TP_STRANGE)
                return ast->nodes[n].info.type_info = right_type;
            if (right_to_left == TP_STRANGE)
                return ast->nodes[n].info.type_info = left_type;

            /* Fall back to conversions with loss of info, but log a warning */
            if (left_to_right == TP_LOSS_OF_INFO)
            {
                log_flc(
                    "{w:semantic warning:} ",
                    source_filename,
                    source.text.data,
                    ast->nodes[n].info.location,
                    "Converting from {quote:%s} to {quote:%s} results in loss "
                    "of information.\n",
                    type_to_db_name(left_type),
                    type_to_db_name(right_type));
                log_binop_excerpt(
                    source_filename,
                    source.text.data,
                    ast->nodes[lhs].info.location,
                    ast->nodes[n].info.location,
                    ast->nodes[rhs].info.location,
                    type_to_db_name(left_type),
                    type_to_db_name(right_type));
                return ast->nodes[n].info.type_info = right_type;
            }
            if (right_to_left == TP_LOSS_OF_INFO)
            {
                log_flc(
                    "{w:semantic warning:} ",
                    source_filename,
                    source.text.data,
                    ast->nodes[n].info.location,
                    "Converting from {quote:%s} to {quote:%s} results in loss "
                    "of information.\n",
                    type_to_db_name(right_type),
                    type_to_db_name(left_type));
                log_binop_excerpt(
                    source_filename,
                    source.text.data,
                    ast->nodes[lhs].info.location,
                    ast->nodes[n].info.location,
                    ast->nodes[rhs].info.location,
                    type_to_db_name(left_type),
                    type_to_db_name(right_type));
                return ast->nodes[n].info.type_info = left_type;
            }

            /* Invalid conversion */
            log_flc(
                "{e:semantic error:} ",
                source_filename,
                source.text.data,
                ast->nodes[n].info.location,
                "Cannot convert from {quote:%s} to {quote:%s}\n",
                type_to_db_name(left_type),
                type_to_db_name(right_type));
            log_binop_excerpt(
                source_filename,
                source.text.data,
                ast->nodes[lhs].info.location,
                ast->nodes[n].info.location,
                ast->nodes[rhs].info.location,
                type_to_db_name(left_type),
                type_to_db_name(right_type));
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
    {
        if (ast->nodes[n].info.type_info == TYPE_VOID)
            if (resolve_expression(ast, n, cmds, source_filename, source)
                == (enum type) - 1)
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
            case AST_STRING_LITERAL: break;
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
const struct semantic_check semantic_type_check_expressions
    = {depends, type_check_expressions};
