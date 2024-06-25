#include "odb-compiler/ast/ast.h"
#include "odb-compiler/sdk/type.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-sdk/config.h"
#include "odb-sdk/log.h"
#include <assert.h>

enum type
type_check_and_cast_binop_pow(
    struct ast*      ast,
    ast_id           op,
    const char*      source_filename,
    struct db_source source)
{
    ODBSDK_DEBUG_ASSERT(op > -1, (void)0);
    ODBSDK_DEBUG_ASSERT(
        ast->nodes[op].info.node_type == AST_BINOP,
        log_semantic_err("type: %d\n", ast->nodes[op].info.node_type));

    ast_id lhs = ast->nodes[op].binop.left;
    ast_id rhs = ast->nodes[op].binop.right;

    enum type base_type = ast->nodes[lhs].info.type_info;
    enum type exp_type = ast->nodes[rhs].info.type_info;

    /*
     * The supported instructions are as of this writing:
     *   powi(f32, i32)
     *   powi(f64, i32)
     *   pow(f32, f32)
     *   pow(f64, f64)
     */
    enum type base_target_type
        = base_type == TYPE_DOUBLE ? TYPE_DOUBLE : TYPE_FLOAT;
    enum type exp_target_type = exp_type == TYPE_DOUBLE  ? TYPE_DOUBLE
                                : exp_type == TYPE_FLOAT ? TYPE_FLOAT
                                                         : TYPE_INTEGER;
    /*
     * It makes sense to prioritize the LHS type higher than the
     * RHS type. For example, if the LHS is a f32, but the RHS
     * is a f64, then the RHS should be cast to a f32.
     */
    if (exp_target_type != TYPE_INTEGER)
        exp_target_type = base_target_type;

    if (base_type != base_target_type)
    {
        ast_id cast_lhs;
        int    gutter;

        switch (type_promote(base_type, base_target_type))
        {
            case TP_ALLOW: break;
            case TP_TRUENESS:
            case TP_DISALLOW:
                log_flc_err(
                    source_filename,
                    source.text.data,
                    ast->nodes[lhs].info.location,
                    "Incompatible base type {emph1:%s} can't be converted to "
                    "{emph2:%s}.\n",
                    type_to_db_name(base_type),
                    type_to_db_name(base_target_type));
                gutter = log_excerpt_2(
                    source.text.data,
                    ast->nodes[lhs].info.location,
                    ast->nodes[op].binop.op_location,
                    type_to_db_name(base_type),
                    "");
                log_excerpt_note(
                    gutter,
                    "The base can be a {emph2:%s} or {emph2:%s}.\n",
                    type_to_db_name(TYPE_FLOAT),
                    type_to_db_name(TYPE_DOUBLE));
                return TYPE_INVALID;

            case TP_TRUNCATE:
                log_flc_warn(
                    source_filename,
                    source.text.data,
                    ast->nodes[lhs].info.location,
                    "Base value is truncated when converting from {emph1:%s} to "
                    "{emph2:%s} in binary expression.\n",
                    type_to_db_name(base_type),
                    type_to_db_name(base_target_type));
                gutter = log_excerpt_binop(
                    source.text.data,
                    ast->nodes[lhs].info.location,
                    ast->nodes[op].binop.op_location,
                    ast->nodes[rhs].info.location,
                    type_to_db_name(base_type),
                    type_to_db_name(exp_type));
                log_excerpt_note(
                    gutter,
                    "The base can be a {emph2:%s} or {emph2:%s}.\n",
                    type_to_db_name(TYPE_FLOAT),
                    type_to_db_name(TYPE_DOUBLE));
                break;

            case TP_BOOL_PROMOTION:
            case TP_INT_TO_FLOAT:
                log_flc_warn(
                    source_filename,
                    source.text.data,
                    ast->nodes[lhs].info.location,
                    "Implicit conversion of base from {emph1:%s} to {emph2:%s}.\n",
                    type_to_db_name(base_type),
                    type_to_db_name(base_target_type));
                gutter = log_excerpt_2(
                    source.text.data,
                    ast->nodes[lhs].info.location,
                    ast->nodes[op].binop.op_location,
                    type_to_db_name(base_type),
                    "");
                log_excerpt_note(
                    gutter,
                    "The base can be a {emph2:%s} or {emph2:%s}\n",
                    type_to_db_name(TYPE_FLOAT),
                    type_to_db_name(TYPE_DOUBLE));
                break;
        }

        /* Cast is required, insert one in the AST */
        cast_lhs = ast_cast(
            ast, lhs, base_target_type, ast->nodes[lhs].info.location);
        if (cast_lhs == TYPE_INVALID)
            return TYPE_INVALID;
        ast->nodes[op].binop.left = cast_lhs;
    }

    if (exp_type != exp_target_type)
    {
        ast_id cast_rhs;
        int    gutter;

        switch (type_promote(exp_type, exp_target_type))
        {
            case TP_ALLOW: break;
            case TP_TRUENESS:
            case TP_DISALLOW:
                log_flc_err(
                    source_filename,
                    source.text.data,
                    ast->nodes[rhs].info.location,
                    "Incompatible exponent type {emph1:%s} can't be converted to "
                    "{emph2:%s}.\n",
                    type_to_db_name(exp_type),
                    type_to_db_name(exp_target_type));
                gutter = log_excerpt_binop(
                    source.text.data,
                    ast->nodes[lhs].info.location,
                    ast->nodes[op].binop.op_location,
                    ast->nodes[rhs].info.location,
                    "",
                    type_to_db_name(exp_type));
                log_excerpt_note(
                    gutter,
                    "The exponent can be an {emph2:%s}, {emph2:%s} or {emph2:%s}.\n",
                    type_to_db_name(TYPE_INTEGER),
                    type_to_db_name(TYPE_FLOAT),
                    type_to_db_name(TYPE_DOUBLE));
                return TYPE_INVALID;

            case TP_TRUNCATE:
                log_flc_warn(
                    source_filename,
                    source.text.data,
                    ast->nodes[rhs].info.location,
                    "Exponent value is truncated when converting from {emph2:%s} "
                    "to {emph1:%s}.\n",
                    type_to_db_name(exp_type),
                    type_to_db_name(exp_target_type));
                gutter = log_excerpt_binop(
                    source.text.data,
                    ast->nodes[lhs].info.location,
                    ast->nodes[op].binop.op_location,
                    ast->nodes[rhs].info.location,
                    exp_target_type == TYPE_FLOAT ? type_to_db_name(TYPE_FLOAT)
                                                  : "",
                    type_to_db_name(exp_type));
                if (exp_target_type == TYPE_FLOAT)
                    log_excerpt_note(
                        gutter,
                        "The exponent is always converted to the same type as "
                        "the base when using floating point exponents.\n");
                if (exp_target_type == TYPE_INTEGER)
                    log_excerpt_note(
                        gutter,
                        "{emph2:INTEGER} is the largest possible integral type "
                        "for exponents.\n");
                log_excerpt_note(
                    gutter,
                    "The exponent can be an {emph2:%s}, {emph2:%s} or {emph2:%s}.\n",
                    type_to_db_name(TYPE_INTEGER),
                    type_to_db_name(TYPE_FLOAT),
                    type_to_db_name(TYPE_DOUBLE));
                break;

            case TP_INT_TO_FLOAT:
            case TP_BOOL_PROMOTION:
                log_flc_warn(
                    source_filename,
                    source.text.data,
                    ast->nodes[rhs].info.location,
                    "Implicit conversion of exponent from {emph2:%s} to "
                    "{emph1:%s}.\n",
                    type_to_db_name(exp_type),
                    type_to_db_name(exp_target_type));
                gutter = log_excerpt_binop(
                    source.text.data,
                    ast->nodes[lhs].info.location,
                    ast->nodes[op].binop.op_location,
                    ast->nodes[rhs].info.location,
                    "",
                    type_to_db_name(exp_type));
                if (exp_target_type == TYPE_FLOAT)
                    log_excerpt_note(
                        gutter,
                        "The exponent needs to be the same type as the base "
                        "when working with floating point types.\n");
                if (exp_type == TYPE_DOUBLE_INTEGER || exp_type == TYPE_DWORD)
                    log_excerpt_note(
                        gutter,
                        "{emph2:INTEGER} is the largest possible integral type "
                        "for exponents.\n");
                log_excerpt_note(
                    gutter,
                    "The exponent can be an {emph2:%s}, {emph2:%s} or {emph2:%s}.\n",
                    type_to_db_name(TYPE_INTEGER),
                    type_to_db_name(TYPE_FLOAT),
                    type_to_db_name(TYPE_DOUBLE));
                break;
        }

        /* Cast is required, insert one in the AST */
        cast_rhs = ast_cast(
            ast, rhs, exp_target_type, ast->nodes[rhs].info.location);
        if (cast_rhs == TYPE_INVALID)
            return TYPE_INVALID;
        ast->nodes[op].binop.right = cast_rhs;
    }

    /* The result type is the same as LHS */
    return ast->nodes[op].binop.info.type_info = base_target_type;
}
