#include "odb-compiler/ast/ast.h"
#include "odb-compiler/sdk/type.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-util/config.h"
#include "odb-util/log.h"
#include <assert.h>

static void
log_narrow_binop(
    struct ast* ast,
    ast_id      op,
    ast_id      source_node,
    ast_id      target_node,
    const char* source_filename,
    const char* source_text)
{
    ast_id    lhs = ast->nodes[op].binop.left;
    ast_id    rhs = ast->nodes[op].binop.right;
    enum type source_type = ast->nodes[source_node].info.type_info;
    enum type target_type = ast->nodes[target_node].info.type_info;

    log_flc_warn(
        source_filename,
        source_text,
        ast->nodes[op].info.location,
        "Value is truncated when converting from {emph1:%s} to {emph2:%s} in "
        "binary expression.\n",
        type_to_db_name(source_type),
        type_to_db_name(target_type));
    log_excerpt_binop(
        source_text,
        ast->nodes[lhs].info.location,
        ast->nodes[op].binop.op_location,
        ast->nodes[rhs].info.location,
        type_to_db_name(lhs == source_node ? source_type : target_type),
        type_to_db_name(lhs == source_node ? target_type : source_type));
}

static void
log_implicit_binop(
    struct ast* ast,
    ast_id      op,
    ast_id      source_node,
    ast_id      target_node,
    const char* source_filename,
    const char* source_text)
{
    ast_id    lhs = ast->nodes[op].binop.left;
    ast_id    rhs = ast->nodes[op].binop.right;
    enum type source_type = ast->nodes[source_node].info.type_info;
    enum type target_type = ast->nodes[target_node].info.type_info;

    log_flc_warn(
        source_filename,
        source_text,
        ast->nodes[op].info.location,
        "Implicit conversion from {emph1:%s} to {emph2:%s} in binary "
        "expression.\n",
        type_to_db_name(source_type),
        type_to_db_name(target_type));
    log_excerpt_binop(
        source_text,
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
    const char* source_text)
{
    ast_id    lhs = ast->nodes[op].binop.left;
    ast_id    rhs = ast->nodes[op].binop.right;
    enum type source_type = ast->nodes[source_node].info.type_info;
    enum type target_type = ast->nodes[op].info.type_info;

    log_flc_err(
        source_filename,
        source_text,
        ast->nodes[op].info.location,
        "Invalid conversion from {emph1:%s} to {emph2:%s} in binary "
        "expression. "
        "Types are incompatible.\n",
        type_to_db_name(source_type),
        type_to_db_name(target_type));
    log_excerpt_binop(
        source_text,
        ast->nodes[lhs].info.location,
        ast->nodes[op].binop.op_location,
        ast->nodes[rhs].info.location,
        type_to_db_name(lhs == source_node ? source_type : target_type),
        type_to_db_name(lhs == source_node ? target_type : source_type));
}

enum type
type_check_binop_symmetric(
    struct ast*      ast,
    ast_id           op,
    const char*      source_filename,
    const char* source_text)
{
    ODBUTIL_DEBUG_ASSERT(op > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast->nodes[op].info.node_type == AST_BINOP,
        log_semantic_err("type: %d\n", ast->nodes[op].info.node_type));

    ast_id lhs = ast->nodes[op].binop.left;
    ast_id rhs = ast->nodes[op].binop.right;

    enum type target_type = TYPE_INVALID;
    enum type lhs_type = ast->nodes[lhs].info.type_info;
    enum type rhs_type = ast->nodes[rhs].info.type_info;

    /*
     * These operations require that both LHS and RHS have the same
     * type. The result type will be the "wider" of the two types.
     */
    enum type_conversion_result left_to_right = type_convert(lhs_type, rhs_type);
    enum type_conversion_result right_to_left = type_convert(rhs_type, lhs_type);

    if (left_to_right == TC_ALLOW)
        target_type = rhs_type;
    else if (right_to_left == TC_ALLOW)
        target_type = lhs_type;

    else if (
        left_to_right == TC_INT_TO_FLOAT || left_to_right == TC_BOOL_PROMOTION)
    {
        target_type = rhs_type;
        log_implicit_binop(ast, op, lhs, rhs, source_filename, source_text);
    }
    else if (
        right_to_left == TC_INT_TO_FLOAT || right_to_left == TC_BOOL_PROMOTION)
    {
        target_type = lhs_type;
        log_implicit_binop(ast, op, rhs, lhs, source_filename, source_text);
    }

    else if (left_to_right == TC_TRUNCATE)
    {
        target_type = rhs_type;
        log_narrow_binop(ast, op, lhs, rhs, source_filename, source_text);
    }
    else if (right_to_left == TC_TRUNCATE)
    {
        target_type = lhs_type;
        log_narrow_binop(ast, op, rhs, lhs, source_filename, source_text);
    }

    else if (left_to_right == TC_TRUENESS)
    {
        target_type = rhs_type;
        log_implicit_binop(ast, op, lhs, rhs, source_filename, source_text);
    }
    else if (right_to_left == TC_TRUENESS)
    {
        target_type = lhs_type;
        log_implicit_binop(ast, op, rhs, lhs, source_filename, source_text);
    }

    /* Invalid conversion */
    if (target_type == TYPE_INVALID)
    {
        log_error_binop(ast, lhs, op, source_filename, source_text);
        return TYPE_INVALID;
    }

    /* Insert casts to result type, if necessary */
    if (ast->nodes[lhs].info.type_info != target_type)
    {
        ast_id cast_lhs
            = ast_cast(ast, lhs, target_type, ast->nodes[lhs].info.location);
        if (cast_lhs == TYPE_INVALID)
            return TYPE_INVALID;
        ast->nodes[op].binop.left = cast_lhs;
    }

    if (ast->nodes[rhs].info.type_info != target_type)
    {
        ast_id cast_rhs
            = ast_cast(ast, rhs, target_type, ast->nodes[rhs].info.location);
        if (cast_rhs == TYPE_INVALID)
            return TYPE_INVALID;
        ast->nodes[op].binop.right = cast_rhs;
    }

    /* Set result type and return success */
    return ast->nodes[op].info.type_info = target_type;
}
