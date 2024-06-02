#include "odb-compiler/ast/ast.h"
#include "odb-compiler/sdk/type.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-sdk/log.h"

#define LOSSY_BINOP                                                            \
    "Value is truncated when converting from {lhs:%s} to {rhs:%s} in binary "  \
    "expression.\n"
#define STRANGE_BINOP                                                          \
    "Strange conversion from {lhs:%s} to {rhs:%s} in binary expression.\n"
#define ERROR_BINOP                                                            \
    "Invalid conversion from {lhs:%s} to {rhs:%s} in binary expression. "      \
    "Types are incompatible.\n"

static void
log_narrow_binop(
    struct ast*      ast,
    ast_id           op,
    ast_id           source_node,
    ast_id           target_node,
    const char*      source_filename,
    struct db_source source)
{
    ast_id    lhs = ast->nodes[op].binop.left;
    ast_id    rhs = ast->nodes[op].binop.right;
    enum type source_type = ast->nodes[source_node].info.type_info;
    enum type target_type = ast->nodes[target_node].info.type_info;

    log_flc(
        "{w:warning:} ",
        source_filename,
        source.text.data,
        ast->nodes[op].info.location,
        LOSSY_BINOP,
        type_to_db_name(source_type),
        type_to_db_name(target_type));
    log_binop_excerpt(
        source_filename,
        source.text.data,
        ast->nodes[lhs].info.location,
        ast->nodes[op].binop.op_location,
        ast->nodes[rhs].info.location,
        type_to_db_name(lhs == source_node ? source_type : target_type),
        type_to_db_name(lhs == source_node ? target_type : source_type));
}

static void
log_strange_binop(
    struct ast*      ast,
    ast_id           op,
    ast_id           source_node,
    ast_id           target_node,
    const char*      source_filename,
    struct db_source source)
{
    ast_id    lhs = ast->nodes[op].binop.left;
    ast_id    rhs = ast->nodes[op].binop.right;
    enum type source_type = ast->nodes[source_node].info.type_info;
    enum type target_type = ast->nodes[target_node].info.type_info;

    log_flc(
        "{w:warning:} ",
        source_filename,
        source.text.data,
        ast->nodes[op].info.location,
        STRANGE_BINOP,
        type_to_db_name(source_type),
        type_to_db_name(target_type));
    log_binop_excerpt(
        source_filename,
        source.text.data,
        ast->nodes[lhs].info.location,
        ast->nodes[op].binop.op_location,
        ast->nodes[rhs].info.location,
        type_to_db_name(lhs == source_node ? source_type : target_type),
        type_to_db_name(lhs == source_node ? target_type : source_type));
}

static void
log_error_binop(
    struct ast*      ast,
    ast_id           source_node,
    ast_id           op,
    const char*      source_filename,
    struct db_source source)
{
    ast_id    lhs = ast->nodes[op].binop.left;
    ast_id    rhs = ast->nodes[op].binop.right;
    enum type source_type = ast->nodes[source_node].info.type_info;
    enum type target_type = ast->nodes[op].info.type_info;

    log_flc(
        "{e:error:} ",
        source_filename,
        source.text.data,
        ast->nodes[op].info.location,
        ERROR_BINOP,
        type_to_db_name(source_type),
        type_to_db_name(target_type));
    log_binop_excerpt(
        source_filename,
        source.text.data,
        ast->nodes[lhs].info.location,
        ast->nodes[op].binop.op_location,
        ast->nodes[rhs].info.location,
        type_to_db_name(lhs == source_node ? source_type : target_type),
        type_to_db_name(lhs == source_node ? target_type : source_type));
}

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
            ast_id    lhs = ast->nodes[n].binop.left;
            ast_id    rhs = ast->nodes[n].binop.right;
            enum type left_type
                = resolve_expression(ast, lhs, cmds, source_filename, source);
            enum type right_type
                = resolve_expression(ast, rhs, cmds, source_filename, source);

            switch (ast->nodes[n].binop.op)
            {
                /*
                 * These operations require that both LHS and RHS have the same
                 * type. The result type will be the "wider" of the two types.
                 */
                case BINOP_ADD:
                case BINOP_SUB:
                case BINOP_MUL:
                case BINOP_DIV:
                case BINOP_MOD: {
                    enum type                  target_type;
                    enum type_promotion_result left_to_right
                        = type_promote(left_type, right_type);
                    enum type_promotion_result right_to_left
                        = type_promote(right_type, left_type);

                    if (left_to_right == TP_ALLOW)
                    {
                        target_type = right_type;
                        goto binop_add_success;
                    }
                    if (right_to_left == TP_ALLOW)
                    {
                        target_type = left_type;
                        goto binop_add_success;
                    }

                    if (left_to_right == TP_STRANGE)
                    {
                        target_type = right_type;
                        log_strange_binop(
                            ast, n, lhs, rhs, source_filename, source);
                        goto binop_add_success;
                    }
                    if (right_to_left == TP_STRANGE)
                    {
                        target_type = left_type;
                        log_strange_binop(
                            ast, n, rhs, lhs, source_filename, source);
                        goto binop_add_success;
                    }

                    if (left_to_right == TP_TRUNCATE)
                    {
                        target_type = right_type;
                        log_narrow_binop(
                            ast, n, lhs, rhs, source_filename, source);
                        goto binop_add_success;
                    }
                    if (right_to_left == TP_TRUNCATE)
                    {
                        target_type = left_type;
                        log_narrow_binop(
                            ast, n, rhs, lhs, source_filename, source);
                        goto binop_add_success;
                    }

                    /* Invalid conversion */
                    log_error_binop(ast, lhs, n, source_filename, source);
                    break;

                binop_add_success:;
                    /* Insert casts to result type, if necessary */
                    if (ast->nodes[lhs].info.type_info != target_type)
                    {
                        ast_id cast_lhs = ast_cast(
                            ast,
                            lhs,
                            target_type,
                            ast->nodes[lhs].info.location);
                        if (cast_lhs < -1)
                            return -1;
                        ast->nodes[n].binop.left = cast_lhs;
                    }

                    if (ast->nodes[rhs].info.type_info != target_type)
                    {
                        ast_id cast_rhs = ast_cast(
                            ast,
                            rhs,
                            target_type,
                            ast->nodes[rhs].info.location);
                        if (cast_rhs < -1)
                            return -1;
                        ast->nodes[n].binop.right = cast_rhs;
                    }

                    /* Set result type and return */
                    return ast->nodes[n].info.type_info = target_type;
                }
                break;

                case BINOP_POW: {
                    /*
                     * The supported instructions are as of this writing:
                     *   powi(f32, i32)
                     *   powi(f64, i32)
                     *   pow(f32, f32)
                     *   pow(f64, f64)
                     */
                    enum type lhs_target_type
                        = left_type == TYPE_DOUBLE ? TYPE_DOUBLE : TYPE_FLOAT;
                    enum type rhs_target_type
                        = right_type == TYPE_DOUBLE  ? TYPE_DOUBLE
                          : right_type == TYPE_FLOAT ? TYPE_FLOAT
                                                     : TYPE_INTEGER;
                    /*
                     * It makes sense to prioritize the LHS type higher than the
                     * RHS type. For example, if the LHS is a f32, but the RHS
                     * is a f64, then the RHS should be cast to a f32.
                     */
                    if (rhs_target_type != TYPE_INTEGER)
                        rhs_target_type = lhs_target_type;

                    if (left_type != lhs_target_type)
                    {
                        ast_id cast_lhs;
                        switch (type_promote(left_type, lhs_target_type))
                        {
                            case TP_ALLOW: break;
                            case TP_STRANGE:
                                log_flc(
                                    "{w:warning:} ",
                                    source_filename,
                                    source.text.data,
                                    ast->nodes[lhs].info.location,
                                    STRANGE_BINOP,
                                    type_to_db_name(left_type),
                                    type_to_db_name(lhs_target_type));
                                log_excerpt2(
                                    source_filename,
                                    source.text.data,
                                    ast->nodes[lhs].info.location,
                                    ast->nodes[n].binop.op_location,
                                    type_to_db_name(left_type),
                                    "");
                                log_note(
                                    "",
                                    "The left operand (base) must be a "
                                    "{rhs:%s} or {rhs:%s}\n",
                                    type_to_db_name(TYPE_FLOAT),
                                    type_to_db_name(TYPE_DOUBLE));
                                break;

                            case TP_TRUNCATE:
                                log_flc(
                                    "{w:warning:} ",
                                    source_filename,
                                    source.text.data,
                                    ast->nodes[lhs].info.location,
                                    LOSSY_BINOP,
                                    type_to_db_name(left_type),
                                    type_to_db_name(lhs_target_type));
                                log_excerpt2(
                                    source_filename,
                                    source.text.data,
                                    ast->nodes[lhs].info.location,
                                    ast->nodes[n].binop.op_location,
                                    type_to_db_name(left_type),
                                    "");
                                log_note(
                                    "",
                                    "The left operand (base) must be a "
                                    "{rhs:%s} or {rhs:%s}\n",
                                    type_to_db_name(TYPE_FLOAT),
                                    type_to_db_name(TYPE_DOUBLE));
                                break;

                            case TP_DISALLOW:
                                log_flc(
                                    "{e:error:} ",
                                    source_filename,
                                    source.text.data,
                                    ast->nodes[lhs].info.location,
                                    ERROR_BINOP,
                                    type_to_db_name(left_type),
                                    type_to_db_name(lhs_target_type));
                                log_excerpt2(
                                    source_filename,
                                    source.text.data,
                                    ast->nodes[lhs].info.location,
                                    ast->nodes[n].binop.op_location,
                                    type_to_db_name(left_type),
                                    "");
                                log_note(
                                    "",
                                    "The left operand (base) must be a "
                                    "{rhs:%s} or {rhs:%s}\n",
                                    type_to_db_name(TYPE_FLOAT),
                                    type_to_db_name(TYPE_DOUBLE));
                                goto binop_pow_error;
                        }

                        /* Cast is required, insert one in the AST */
                        cast_lhs = ast_cast(
                            ast,
                            lhs,
                            lhs_target_type,
                            ast->nodes[lhs].info.location);
                        if (cast_lhs < -1)
                            goto binop_pow_error;
                        ast->nodes[n].binop.left = cast_lhs;
                    }

                    if (right_type != rhs_target_type)
                    {
                        ast_id cast_rhs;
                        switch (type_promote(right_type, rhs_target_type))
                        {
                            case TP_ALLOW: break;
                            case TP_STRANGE:
                                log_flc(
                                    "{w:warning:} ",
                                    source_filename,
                                    source.text.data,
                                    ast->nodes[rhs].info.location,
                                    STRANGE_BINOP,
                                    type_to_db_name(right_type),
                                    type_to_db_name(rhs_target_type));
                                log_excerpt2(
                                    source_filename,
                                    source.text.data,
                                    ast->nodes[n].binop.op_location,
                                    ast->nodes[rhs].info.location,
                                    "",
                                    type_to_db_name(right_type));
                                log_note(
                                    "",
                                    "The right operand (exponent) must be a "
                                    "{rhs:%s}, {rhs:%s} or {rhs:%s}\n",
                                    type_to_db_name(TYPE_INTEGER),
                                    type_to_db_name(TYPE_FLOAT),
                                    type_to_db_name(TYPE_DOUBLE));
                                break;

                            case TP_TRUNCATE:
                                log_flc(
                                    "{w:warning:} ",
                                    source_filename,
                                    source.text.data,
                                    ast->nodes[rhs].info.location,
                                    LOSSY_BINOP,
                                    type_to_db_name(right_type),
                                    type_to_db_name(rhs_target_type));
                                log_excerpt2(
                                    source_filename,
                                    source.text.data,
                                    ast->nodes[n].binop.op_location,
                                    ast->nodes[rhs].info.location,
                                    "",
                                    type_to_db_name(right_type));
                                log_note(
                                    "",
                                    "The right operand (exponent) must be a "
                                    "{rhs:%s}, {rhs:%s} or {rhs:%s}\n",
                                    type_to_db_name(TYPE_INTEGER),
                                    type_to_db_name(TYPE_FLOAT),
                                    type_to_db_name(TYPE_DOUBLE));
                                break;

                            case TP_DISALLOW:
                                log_flc(
                                    "{e:error:} ",
                                    source_filename,
                                    source.text.data,
                                    ast->nodes[rhs].info.location,
                                    ERROR_BINOP,
                                    type_to_db_name(right_type),
                                    type_to_db_name(rhs_target_type));
                                log_excerpt2(
                                    source_filename,
                                    source.text.data,
                                    ast->nodes[n].binop.op_location,
                                    ast->nodes[rhs].info.location,
                                    "",
                                    type_to_db_name(right_type));
                                log_note(
                                    "",
                                    "The right operand (exponent) must be a "
                                    "{rhs:%s}, {rhs:%s} or {rhs:%s}\n",
                                    type_to_db_name(TYPE_INTEGER),
                                    type_to_db_name(TYPE_FLOAT),
                                    type_to_db_name(TYPE_DOUBLE));
                                goto binop_pow_error;
                        }

                        /* Cast is required, insert one in the AST */
                        cast_rhs = ast_cast(
                            ast,
                            rhs,
                            rhs_target_type,
                            ast->nodes[rhs].info.location);
                        if (cast_rhs < -1)
                            goto binop_pow_error;
                        ast->nodes[n].binop.right = cast_rhs;
                    }

                    /* The result type is the same as LHS */
                    return ast->nodes[n].binop.info.type_info = lhs_target_type;

                binop_pow_error:;
                }
                break;

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
        case AST_CAST: {
            int       expr = ast->nodes[n].cast.expr;
            enum type source_type
                = resolve_expression(ast, expr, cmds, source_filename, source);
            enum type target_type = ast->nodes[n].info.type_info;

            switch (type_promote(source_type, target_type))
            {
                case TP_ALLOW: return target_type;
                case TP_DISALLOW:
                    log_flc(
                        "{e:error:} ",
                        source_filename,
                        source.text.data,
                        ast->nodes[n].info.location,
                        "Cannot cast from {lhs:%s} to {rhs:%s}: Types are "
                        "incompatible\n",
                        type_to_db_name(source_type),
                        type_to_db_name(target_type));
                    log_excerpt2(
                        source_filename,
                        source.text.data,
                        ast->nodes[expr].info.location,
                        ast->nodes[n].info.location,
                        type_to_db_name(source_type),
                        type_to_db_name(target_type));
                    break;

                case TP_TRUNCATE:
                    log_flc(
                        "{w:warning:} ",
                        source_filename,
                        source.text.data,
                        ast->nodes[n].info.location,
                        "Value is truncated when converting from {lhs:%s} "
                        "to "
                        "{rhs:%s} in expression\n",
                        type_to_db_name(source_type),
                        type_to_db_name(target_type));
                    log_excerpt2(
                        source_filename,
                        source.text.data,
                        ast->nodes[expr].info.location,
                        ast->nodes[n].info.location,
                        type_to_db_name(source_type),
                        type_to_db_name(target_type));
                    break;

                case TP_STRANGE:
                    log_flc(
                        "{w:warning:} ",
                        source_filename,
                        source.text.data,
                        ast->nodes[n].info.location,
                        "Strange conversion from {lhs:%s} to {rhs:%s} in "
                        "expression\n",
                        type_to_db_name(source_type),
                        type_to_db_name(target_type));
                    log_excerpt2(
                        source_filename,
                        source.text.data,
                        ast->nodes[expr].info.location,
                        ast->nodes[n].info.location,
                        type_to_db_name(source_type),
                        type_to_db_name(target_type));
                    break;
            }

            return target_type;
        }
        break;
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
const struct semantic_check semantic_type_check_expressions
    = {depends, type_check_expressions};
