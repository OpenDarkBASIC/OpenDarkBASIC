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
    ODBSDK_DEBUG_ASSERT(ast->nodes[op].info.node_type == AST_BINOP);

    ast_id lhs = ast->nodes[op].binop.left;
    ast_id rhs = ast->nodes[op].binop.right;

    enum type lhs_type = ast->nodes[lhs].info.type_info;
    enum type rhs_type = ast->nodes[rhs].info.type_info;

    /*
     * The supported instructions are as of this writing:
     *   powi(f32, i32)
     *   powi(f64, i32)
     *   pow(f32, f32)
     *   pow(f64, f64)
     */
    enum type lhs_target_type
        = lhs_type == TYPE_DOUBLE ? TYPE_DOUBLE : TYPE_FLOAT;
    enum type rhs_target_type = rhs_type == TYPE_DOUBLE  ? TYPE_DOUBLE
                                : rhs_type == TYPE_FLOAT ? TYPE_FLOAT
                                                         : TYPE_INTEGER;
    /*
     * It makes sense to prioritize the LHS type higher than the
     * RHS type. For example, if the LHS is a f32, but the RHS
     * is a f64, then the RHS should be cast to a f32.
     */
    if (rhs_target_type != TYPE_INTEGER)
        rhs_target_type = lhs_target_type;

    if (lhs_type != lhs_target_type)
    {
        ast_id cast_lhs;
        int    gutter;

        switch (type_promote(lhs_type, lhs_target_type))
        {
            case TP_ALLOW: break;
            case TP_STRANGE:
                log_flc(
                    "{w:warning:} ",
                    source_filename,
                    source.text.data,
                    ast->nodes[lhs].info.location,
                    "Strange conversion of base value from {lhs:%s} to "
                    "{rhs:%s}.\n",
                    type_to_db_name(lhs_type),
                    type_to_db_name(lhs_target_type));
                gutter = log_excerpt2(
                    source_filename,
                    source.text.data,
                    ast->nodes[lhs].info.location,
                    ast->nodes[op].binop.op_location,
                    type_to_db_name(lhs_type),
                    "");
                log_excerpt_note(
                    gutter,
                    "The base can be an {rhs:%s} or {rhs:%s}\n",
                    type_to_db_name(TYPE_FLOAT),
                    type_to_db_name(TYPE_DOUBLE));
                break;

            case TP_TRUNCATE:
                log_flc(
                    "{w:warning:} ",
                    source_filename,
                    source.text.data,
                    ast->nodes[lhs].info.location,
                    "Base value is truncated when converting from {lhs:%s} to "
                    "{rhs:%s} in binary expression.\n",
                    type_to_db_name(lhs_type),
                    type_to_db_name(lhs_target_type));
                gutter = log_binop_excerpt(
                    source_filename,
                    source.text.data,
                    ast->nodes[lhs].info.location,
                    ast->nodes[op].binop.op_location,
                    ast->nodes[rhs].info.location,
                    type_to_db_name(lhs_type),
                    type_to_db_name(rhs_type));
                log_excerpt_note(
                    gutter,
                    "The base can be an {rhs:%s} or {rhs:%s}.\n",
                    type_to_db_name(TYPE_FLOAT),
                    type_to_db_name(TYPE_DOUBLE));
                break;

            case TP_DISALLOW:
                log_flc(
                    "{e:error:} ",
                    source_filename,
                    source.text.data,
                    ast->nodes[lhs].info.location,
                    "Incompatible base type {lhs:%s} can't be converted to "
                    "{rhs:%s}.\n",
                    type_to_db_name(lhs_type),
                    type_to_db_name(lhs_target_type));
                gutter = log_excerpt2(
                    source_filename,
                    source.text.data,
                    ast->nodes[lhs].info.location,
                    ast->nodes[op].binop.op_location,
                    type_to_db_name(lhs_type),
                    "");
                log_excerpt_note(
                    gutter,
                    "The base can be an {rhs:%s} or {rhs:%s}.\n",
                    type_to_db_name(TYPE_FLOAT),
                    type_to_db_name(TYPE_DOUBLE));
                return TYPE_INVALID;
        }

        /* Cast is required, insert one in the AST */
        cast_lhs = ast_cast(
            ast, lhs, lhs_target_type, ast->nodes[lhs].info.location);
        if (cast_lhs == TYPE_INVALID)
            return TYPE_INVALID;
        ast->nodes[op].binop.left = cast_lhs;
    }

    if (rhs_type != rhs_target_type)
    {
        ast_id cast_rhs;
        int    gutter;

        switch (type_promote(rhs_type, rhs_target_type))
        {
            case TP_ALLOW: break;
            case TP_STRANGE:
                log_flc(
                    "{w:warning:} ",
                    source_filename,
                    source.text.data,
                    ast->nodes[rhs].info.location,
                    "Strange conversion of exponent value from {rhs:%s} to "
                    "{lhs:%s}.\n",
                    type_to_db_name(rhs_type),
                    type_to_db_name(rhs_target_type));
                gutter = log_binop_excerpt(
                    source_filename,
                    source.text.data,
                    ast->nodes[lhs].info.location,
                    ast->nodes[op].binop.op_location,
                    ast->nodes[rhs].info.location,
                    "",
                    type_to_db_name(rhs_type));
                if (rhs_target_type == TYPE_FLOAT)
                    log_excerpt_note(
                        gutter,
                        "The exponent needs to be the same type as the base "
                        "when working with floating point types.\n");
                if (rhs_type == TYPE_LONG || rhs_type == TYPE_DWORD)
                    log_excerpt_note(
                        gutter,
                        "{rhs:INTEGER} is the largest possible integral type "
                        "for exponents.\n");
                log_excerpt_note(
                    gutter,
                    "The exponent can be an {rhs:%s}, {rhs:%s} or {rhs:%s}.\n",
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
                    "Exponent value is truncated when converting from {rhs:%s} "
                    "to {lhs:%s}.\n",
                    type_to_db_name(rhs_type),
                    type_to_db_name(rhs_target_type));
                gutter = log_binop_excerpt(
                    source_filename,
                    source.text.data,
                    ast->nodes[lhs].info.location,
                    ast->nodes[op].binop.op_location,
                    ast->nodes[rhs].info.location,
                    rhs_target_type == TYPE_FLOAT ? type_to_db_name(TYPE_FLOAT)
                                                  : "",
                    type_to_db_name(rhs_type));
                if (rhs_target_type == TYPE_FLOAT)
                    log_excerpt_note(
                        gutter,
                        "The exponent is always converted to the same type as "
                        "the base when using floating point exponents.\n");
                if (rhs_target_type == TYPE_INTEGER)
                    log_excerpt_note(
                        gutter,
                        "{rhs:INTEGER} is the largest possible integral type "
                        "for exponents.\n");
                log_excerpt_note(
                    gutter,
                    "The exponent can be an {rhs:%s}, {rhs:%s} or {rhs:%s}.\n",
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
                    "Incompatible exponent type {lhs:%s} can't be converted to "
                    "{rhs:%s}.\n",
                    type_to_db_name(rhs_type),
                    type_to_db_name(rhs_target_type));
                gutter = log_binop_excerpt(
                    source_filename,
                    source.text.data,
                    ast->nodes[lhs].info.location,
                    ast->nodes[op].binop.op_location,
                    ast->nodes[rhs].info.location,
                    "",
                    type_to_db_name(rhs_type));
                log_excerpt_note(
                    gutter,
                    "The exponent can be an {rhs:%s}, {rhs:%s} or {rhs:%s}.\n",
                    type_to_db_name(TYPE_INTEGER),
                    type_to_db_name(TYPE_FLOAT),
                    type_to_db_name(TYPE_DOUBLE));
                return TYPE_INVALID;
        }

        /* Cast is required, insert one in the AST */
        cast_rhs = ast_cast(
            ast, rhs, rhs_target_type, ast->nodes[rhs].info.location);
        if (cast_rhs == TYPE_INVALID)
            return TYPE_INVALID;
        ast->nodes[op].binop.right = cast_rhs;
    }

    /* The result type is the same as LHS */
    return ast->nodes[op].binop.info.type_info = lhs_target_type;
}
