#include "odb-compiler/ast/ast.h"
#include "odb-compiler/messages/messages.h"
#include "odb-util/log.h"

int
err_assignment_incompatible_types(
    const struct ast* ast,
    ast_id            ass,
    ast_id            orig_decl,
    const char*       filename,
    const char*       source)
{
    int              gutter;
    ast_id           lhs = ast->nodes[ass].assignment.lvalue;
    ast_id           rhs = ast->nodes[ass].assignment.expr;
    struct utf8_span orig_name = ast->nodes[orig_decl].identifier.name;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, ass) == AST_ASSIGNMENT,
        log_err("", "type: %d\n", ast_node_type(ast, ass)));
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, orig_decl) == AST_IDENTIFIER,
        log_err("", "type: %d\n", ast_node_type(ast, orig_decl)));

    log_flc_err(
        filename,
        source,
        ast_loc(ast, rhs),
        "Cannot assign {emph2:%s} to {emph1:%s}. Types are incompatible.\n",
        type_to_db_name(ast_type_info(ast, rhs)),
        type_to_db_name(ast_type_info(ast, lhs)));
    gutter = log_excerpt_binop(
        source,
        ast_loc(ast, lhs),
        ast->nodes[ass].assignment.op_location,
        ast_loc(ast, rhs),
        type_to_db_name(ast_type_info(ast, lhs)),
        type_to_db_name(ast_type_info(ast, rhs)));
    log_excerpt_note(
        gutter,
        "{emph1:%.*s} was previously declared as {emph1:%s} at ",
        orig_name.len,
        source + orig_name.off,
        type_to_db_name(ast_type_info(ast, lhs)));
    log_flc("", filename, source, ast_loc(ast, orig_decl), "\n");
    log_excerpt_1(
        source,
        ast_loc(ast, orig_decl),
        type_to_db_name(ast_type_info(ast, lhs)));

    return -1;
}

int
err_binop_incompatible_types(
    const struct ast* ast,
    ast_id            source_node,
    ast_id            op,
    const char*       filename,
    const char*       source)
{
    ast_id    lhs = ast->nodes[op].binop.left;
    ast_id    rhs = ast->nodes[op].binop.right;
    enum type source_type = ast_type_info(ast, source_node);
    enum type target_type = ast_type_info(ast, op);

    log_flc_err(
        filename,
        source,
        ast_loc(ast, op),
        "Invalid conversion from {emph1:%s} to {emph2:%s} in binary "
        "expression. Types are incompatible.\n",
        type_to_db_name(source_type),
        type_to_db_name(target_type));
    log_excerpt_binop(
        source,
        ast_loc(ast, lhs),
        ast->nodes[op].binop.op_location,
        ast_loc(ast, rhs),
        type_to_db_name(lhs == source_node ? source_type : target_type),
        type_to_db_name(lhs == source_node ? target_type : source_type));

    return -1;
}

int
err_binop_pow_incompatible_base_type(
    const struct ast* ast,
    ast_id            op,
    enum type         base_type,
    enum type         target_type,
    const char*       filename,
    const char*       source)
{
    int    gutter;
    ast_id base = ast->nodes[op].binop.left;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, op) == AST_BINOP,
        log_err("", "type: %d\n", ast_node_type(ast, op)));
    ODBUTIL_DEBUG_ASSERT(
        ast->nodes[op].binop.op == BINOP_POW,
        log_err("", "op: %d\n", ast->nodes[op].binop.op));

    log_flc_err(
        filename,
        source,
        ast_loc(ast, base),
        "Incompatible base type {emph1:%s} can't be converted to {emph2:%s}.\n",
        type_to_db_name(base_type),
        type_to_db_name(target_type));
    gutter = log_excerpt_2(
        source,
        ast_loc(ast, base),
        ast->nodes[op].binop.op_location,
        type_to_db_name(base_type),
        "");
    log_excerpt_note(
        gutter,
        "The base can be a {emph2:%s} or {emph2:%s}.\n",
        type_to_db_name(TYPE_F32),
        type_to_db_name(TYPE_F64));

    return -1;
}

int
err_binop_pow_incompatible_exponent_type(
    const struct ast* ast,
    ast_id            op,
    enum type         exp_type,
    enum type         target_type,
    const char*       filename,
    const char*       source)
{
    int    gutter;
    ast_id base = ast->nodes[op].binop.left;
    ast_id exp = ast->nodes[op].binop.right;

    log_flc_err(
        filename,
        source,
        ast_loc(ast, exp),
        "Incompatible exponent type {emph1:%s} can't be converted to "
        "{emph2:%s}.\n",
        type_to_db_name(exp_type),
        type_to_db_name(target_type));
    gutter = log_excerpt_binop(
        source,
        ast_loc(ast, base),
        ast->nodes[op].binop.op_location,
        ast_loc(ast, exp),
        "",
        type_to_db_name(exp_type));
    log_excerpt_note(
        gutter,
        "The exponent can be an {emph2:%s}, {emph2:%s} or {emph2:%s}.\n",
        type_to_db_name(TYPE_I32),
        type_to_db_name(TYPE_F32),
        type_to_db_name(TYPE_F64));

    return -1;
}

int
err_boolean_invalid_evaluation(
    const struct ast* ast,
    ast_id            expr,
    const char*       filename,
    const char*       source)
{
    log_flc_err(
        filename,
        source,
        ast_loc(ast, expr),
        "Cannot evaluate {emph1:%s} as a boolean expression.\n",
        type_to_db_name(ast_type_info(ast, expr)));
    log_excerpt_1(
        source, ast_loc(ast, expr), type_to_db_name(ast_type_info(ast, expr)));

    return -1;
}

int
err_cast_incompatible_types(
    const struct ast* ast,
    ast_id            cast,
    const char*       filename,
    const char*       source)
{
    ast_id    expr = ast->nodes[cast].cast.expr;
    enum type source_type = ast_type_info(ast, expr);
    enum type target_type = ast_type_info(ast, cast);

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, cast) == AST_CAST,
        log_err("", "type: %d\n", ast_node_type(ast, cast)));

    log_flc_err(
        filename,
        source,
        ast_loc(ast, cast),
        "Cannot cast from {emph1:%s} to {emph2:%s}: Types are incompatible\n",
        type_to_db_name(source_type),
        type_to_db_name(target_type));
    log_excerpt_2(
        source,
        ast_loc(ast, expr),
        ast_loc(ast, cast),
        type_to_db_name(source_type),
        type_to_db_name(target_type));

    return -1;
}

int
err_initialization_incompatible_types(
    const struct ast* ast, ast_id ass, const char* filename, const char* source)
{
    ast_id lhs = ast->nodes[ass].assignment.lvalue;
    ast_id rhs = ast->nodes[ass].assignment.expr;

    log_flc_err(
        filename,
        source,
        ast_loc(ast, rhs),
        "Cannot assign {emph2:%s} to {emph1:%s}. Types are incompatible.\n",
        type_to_db_name(ast_type_info(ast, rhs)),
        type_to_db_name(ast_type_info(ast, lhs)));
    log_excerpt_binop(
        source,
        ast_loc(ast, lhs),
        ast->nodes[ass].assignment.op_location,
        ast_loc(ast, rhs),
        type_to_db_name(ast_type_info(ast, lhs)),
        type_to_db_name(ast_type_info(ast, rhs)));

    return -1;
}

int
err_loop_cont(
    const struct ast* ast,
    ast_id            cont,
    ast_id            first_loop,
    const char*       filename,
    const char*       source)
{
    int gutter;
    if (first_loop == -1)
    {
        log_flc_err(
            filename,
            source,
            ast_loc(ast, cont),
            "CONTINUE statement must be inside a loop.\n");
        log_excerpt_1(source, ast_loc(ast, cont), "");
    }
    else
    {
        struct utf8_span name = ast->nodes[first_loop].loop.name.len
                                    ? ast->nodes[first_loop].loop.name
                                : ast->nodes[first_loop].loop.implicit_name.len
                                    ? ast->nodes[first_loop].loop.implicit_name
                                    : empty_utf8_span();
        log_flc_err(
            filename,
            source,
            ast->nodes[cont].cont.name,
            "Unknown loop name referenced in CONTINUE statement.\n");
        gutter = log_excerpt_1(source, ast->nodes[cont].cont.name, "");
        if (name.len)
        {
            log_excerpt_help(
                gutter,
                "Did you mean {quote:%.*s}?\n",
                name.len,
                source + name.off);
            log_excerpt_1(source, name, "");
        }
    }
    return -1;
}

int
err_loop_exit_not_inside_loop(
    const struct ast* ast,
    ast_id            exit,
    const char*       filename,
    const char*       source)
{
    log_flc_err(
        filename,
        source,
        ast_loc(ast, exit),
        "EXIT statement must be inside a loop.\n");
    log_excerpt_1(source, ast_loc(ast, exit), "");

    return -1;
}

int
err_loop_exit_unknown_name(
    const struct ast* ast,
    ast_id            exit,
    ast_id            first_loop,
    const char*       filename,
    const char*       source)
{
    int              gutter;
    struct utf8_span name;

    if (ast->nodes[first_loop].loop.name.len > 0)
        name = ast->nodes[first_loop].loop.name;
    else if (ast->nodes[first_loop].loop.implicit_name.len > 0)
        name = ast->nodes[first_loop].loop.implicit_name;
    else
        name = empty_utf8_span();

    log_flc_err(
        filename,
        source,
        ast->nodes[exit].loop_exit.name,
        "Unknown loop name referenced in EXIT statement.\n");
    gutter = log_excerpt_1(source, ast->nodes[exit].loop_exit.name, "");
    if (name.len)
    {
        log_excerpt_help(
            gutter,
            "Did you mean {quote:%.*s}?\n",
            name.len,
            source + name.off);
        log_excerpt_1(source, name, "");
    }

    return -1;
}

int
err_loop_for_unknown_direction(
    const struct ast* ast,
    ast_id            begin,
    ast_id            end,
    ast_id            step,
    const char*       filename,
    const char*       source)
{
    int              gutter;
    struct utf8_span loc1
        = utf8_span_union(ast_loc(ast, begin), ast_loc(ast, end));
    struct utf8_span     loc2 = ast_loc(ast, step);
    struct log_highlight hl[]
        = {{"", "", loc1, LOG_HIGHLIGHT, LOG_MARKERS, 0},
           {"", "", loc2, LOG_HIGHLIGHT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};
    log_flc_err(
        filename, source, loc1, "Unable to determine direction of for-loop.\n");
    gutter = log_excerpt(source, hl);
    log_excerpt_note(
        gutter,
        "The direction a for-loop counts must be known at compile-time, "
        "because the exit condition depends on it. You can either make the "
        "STEP value a constant, or make both the start and end values "
        "constants.\n");
    return -1;
}

int
err_unterminated_remark(
    struct utf8_span location, const char* filename, const char* source)
{
    log_flc_err(filename, source, location, "Unterminated remark.\n");
    log_excerpt_1(source, location, "Remark starts here.");
    return -1;
}

void
warn_assignment_implicit_conversion(
    const struct ast* ast,
    ast_id            ass,
    ast_id            orig_decl,
    const char*       filename,
    const char*       source)
{
    int              gutter;
    ast_id           lhs = ast->nodes[ass].assignment.lvalue;
    ast_id           rhs = ast->nodes[ass].assignment.expr;
    struct utf8_span orig_name = ast->nodes[orig_decl].identifier.name;

    log_flc_warn(
        filename,
        source,
        ast_loc(ast, rhs),
        "Implicit conversion from {emph2:%s} to {emph1:%s} in assignment.\n",
        type_to_db_name(ast_type_info(ast, rhs)),
        type_to_db_name(ast_type_info(ast, lhs)));
    gutter = log_excerpt_binop(
        source,
        ast_loc(ast, lhs),
        ast->nodes[ass].assignment.op_location,
        ast_loc(ast, rhs),
        type_to_db_name(ast_type_info(ast, lhs)),
        type_to_db_name(ast_type_info(ast, rhs)));
    log_excerpt_note(
        gutter,
        "{emph1:%.*s} was previously declared as {emph1:%s} at ",
        orig_name.len,
        source + orig_name.off,
        type_to_db_name(ast_type_info(ast, lhs)));
    log_flc("", filename, source, ast_loc(ast, orig_decl), "\n");
    log_excerpt_1(
        source,
        ast_loc(ast, orig_decl),
        type_to_db_name(ast_type_info(ast, lhs)));
}

void
warn_assignment_truncation(
    const struct ast* ast,
    ast_id            ass,
    ast_id            orig_decl,
    const char*       filename,
    const char*       source)
{
    int              gutter;
    ast_id           lhs = ast->nodes[ass].assignment.lvalue;
    ast_id           rhs = ast->nodes[ass].assignment.expr;
    struct utf8_span orig_name = ast->nodes[orig_decl].identifier.name;

    log_flc_warn(
        filename,
        source,
        ast_loc(ast, rhs),
        "Value is truncated in conversion from {emph2:%s} to {emph1:%s} in "
        "assignment.\n",
        type_to_db_name(ast_type_info(ast, rhs)),
        type_to_db_name(ast_type_info(ast, lhs)));
    gutter = log_excerpt_binop(
        source,
        ast_loc(ast, lhs),
        ast->nodes[ass].assignment.op_location,
        ast_loc(ast, rhs),
        type_to_db_name(ast_type_info(ast, lhs)),
        type_to_db_name(ast_type_info(ast, rhs)));
    log_excerpt_note(
        gutter,
        "{emph1:%.*s} was previously declared as {emph1:%s} at ",
        orig_name.len,
        source + orig_name.off,
        type_to_db_name(ast_type_info(ast, lhs)));
    log_flc("", filename, source, ast_loc(ast, orig_decl), "\n");
    log_excerpt_1(
        source,
        ast_loc(ast, orig_decl),
        type_to_db_name(ast_type_info(ast, lhs)));
}

void
warn_binop_implicit_conversion(
    const struct ast* ast,
    ast_id            op,
    ast_id            source_node,
    ast_id            target_node,
    const char*       source_filename,
    const char*       source_text)
{
    ast_id    lhs = ast->nodes[op].binop.left;
    ast_id    rhs = ast->nodes[op].binop.right;
    enum type source_type = ast_type_info(ast, source_node);
    enum type target_type = ast_type_info(ast, target_node);

    log_flc_warn(
        source_filename,
        source_text,
        ast_loc(ast, op),
        "Implicit conversion from {emph1:%s} to {emph2:%s} in binary "
        "expression.\n",
        type_to_db_name(source_type),
        type_to_db_name(target_type));
    log_excerpt_binop(
        source_text,
        ast_loc(ast, lhs),
        ast->nodes[op].binop.op_location,
        ast_loc(ast, rhs),
        type_to_db_name(lhs == source_node ? source_type : target_type),
        type_to_db_name(lhs == source_node ? target_type : source_type));
}

void
warn_binop_truncation(
    const struct ast* ast,
    ast_id            op,
    ast_id            source_node,
    ast_id            target_node,
    const char*       filename,
    const char*       source)
{
    ast_id    lhs = ast->nodes[op].binop.left;
    ast_id    rhs = ast->nodes[op].binop.right;
    enum type source_type = ast_type_info(ast, source_node);
    enum type target_type = ast_type_info(ast, target_node);

    log_flc_warn(
        filename,
        source,
        ast_loc(ast, op),
        "Value is truncated when converting from {emph1:%s} to {emph2:%s} in "
        "binary expression.\n",
        type_to_db_name(source_type),
        type_to_db_name(target_type));
    log_excerpt_binop(
        source,
        ast_loc(ast, lhs),
        ast->nodes[op].binop.op_location,
        ast_loc(ast, rhs),
        type_to_db_name(lhs == source_node ? source_type : target_type),
        type_to_db_name(lhs == source_node ? target_type : source_type));
}

void
warn_binop_pow_base_implicit_conversion(
    const struct ast* ast,
    ast_id            op,
    enum type         base_type,
    enum type         target_type,
    const char*       filename,
    const char*       source)
{
    int    gutter;
    ast_id base = ast->nodes[op].binop.left;

    log_flc_warn(
        filename,
        source,
        ast_loc(ast, base),
        "Implicit conversion of base from {emph1:%s} to {emph2:%s}.\n",
        type_to_db_name(base_type),
        type_to_db_name(target_type));
    gutter = log_excerpt_2(
        source,
        ast_loc(ast, base),
        ast->nodes[op].binop.op_location,
        type_to_db_name(base_type),
        "");
    log_excerpt_note(
        gutter,
        "The base can be a {emph2:%s} or {emph2:%s}\n",
        type_to_db_name(TYPE_F32),
        type_to_db_name(TYPE_F64));
}

void
warn_binop_pow_base_truncation(
    const struct ast* ast,
    ast_id            op,
    enum type         base_type,
    enum type         target_type,
    const char*       filename,
    const char*       source)
{
    int    gutter;
    ast_id base = ast->nodes[op].binop.left;
    ast_id exp = ast->nodes[op].binop.right;

    log_flc_warn(
        filename,
        source,
        ast_loc(ast, base),
        "Base value is truncated when converting from {emph1:%s} to {emph2:%s} "
        "in binary expression.\n",
        type_to_db_name(base_type),
        type_to_db_name(target_type));
    gutter = log_excerpt_binop(
        source,
        ast_loc(ast, base),
        ast->nodes[op].binop.op_location,
        ast_loc(ast, exp),
        type_to_db_name(base_type),
        type_to_db_name(ast_type_info(ast, exp)));
    log_excerpt_note(
        gutter,
        "The base can be a {emph2:%s} or {emph2:%s}.\n",
        type_to_db_name(TYPE_F32),
        type_to_db_name(TYPE_F64));
}

void
warn_binop_pow_exponent_implicit_conversion(
    const struct ast* ast,
    ast_id            op,
    enum type         exp_type,
    enum type         target_type,
    const char*       filename,
    const char*       source)
{
    int    gutter;
    ast_id base = ast->nodes[op].binop.left;
    ast_id exp = ast->nodes[op].binop.right;

    log_flc_warn(
        filename,
        source,
        ast_loc(ast, exp),
        "Implicit conversion of exponent from {emph2:%s} to {emph1:%s}.\n",
        type_to_db_name(exp_type),
        type_to_db_name(target_type));
    gutter = log_excerpt_binop(
        source,
        ast_loc(ast, base),
        ast->nodes[op].binop.op_location,
        ast_loc(ast, exp),
        "",
        type_to_db_name(exp_type));
    if (target_type == TYPE_F32)
        log_excerpt_note(
            gutter,
            "The exponent needs to be the same type as the base when working "
            "with floating point types.\n");
    if (exp_type == TYPE_I64 || exp_type == TYPE_U32)
        log_excerpt_note(
            gutter,
            "{emph2:INTEGER} is the largest possible integral type for "
            "exponents.\n");
    log_excerpt_note(
        gutter,
        "The exponent can be an {emph2:%s}, {emph2:%s} or {emph2:%s}.\n",
        type_to_db_name(TYPE_I32),
        type_to_db_name(TYPE_F32),
        type_to_db_name(TYPE_F64));
}
void
warn_binop_pow_exponent_truncation(
    const struct ast* ast,
    ast_id            op,
    enum type         exp_type,
    enum type         target_type,
    const char*       filename,
    const char*       source)
{
    int    gutter;
    ast_id base = ast->nodes[op].binop.left;
    ast_id exp = ast->nodes[op].binop.right;

    log_flc_warn(
        filename,
        source,
        ast_loc(ast, exp),
        "Exponent value is truncated when converting from {emph2:%s} to "
        "{emph1:%s}.\n",
        type_to_db_name(exp_type),
        type_to_db_name(target_type));
    gutter = log_excerpt_binop(
        source,
        ast_loc(ast, base),
        ast->nodes[op].binop.op_location,
        ast_loc(ast, exp),
        target_type == TYPE_F32 ? type_to_db_name(TYPE_F32) : "",
        type_to_db_name(exp_type));
    if (target_type == TYPE_F32)
        log_excerpt_note(
            gutter,
            "The exponent is always converted to the same type as the base "
            "when using floating point exponents.\n");
    if (target_type == TYPE_I32)
        log_excerpt_note(
            gutter,
            "{emph2:INTEGER} is the largest possible integral type for "
            "exponents.\n");
    log_excerpt_note(
        gutter,
        "The exponent can be an {emph2:%s}, {emph2:%s} or {emph2:%s}.\n",
        type_to_db_name(TYPE_I32),
        type_to_db_name(TYPE_F32),
        type_to_db_name(TYPE_F64));
}

void
warn_boolean_implicit_evaluation(
    const struct ast* ast,
    ast_id            expr,
    const char*       filename,
    const char*       source)
{
    int                  gutter;
    struct utf8_span     expr_loc = ast_loc(ast, expr);
    utf8_idx             expr_start = expr_loc.off;
    utf8_idx             expr_end = expr_start + expr_loc.len;
    struct log_highlight hl_int[]
        = {{" <> 0", "", {expr_end, 5}, LOG_INSERT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};
    struct log_highlight hl_float[]
        = {{" <> 0.0f", "", {expr_end, 8}, LOG_INSERT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};
    struct log_highlight hl_double[]
        = {{" <> 0.0", "", {expr_end, 7}, LOG_INSERT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};
    struct log_highlight hl_string[]
        = {{" <> \"\"", "", {expr_end, 6}, LOG_INSERT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};
    log_flc_warn(
        filename,
        source,
        ast_loc(ast, expr),
        "Implicit evaluation of {emph1:%s} as a boolean expression.\n",
        type_to_db_name(ast_type_info(ast, expr)));
    gutter = log_excerpt_1(
        source, ast_loc(ast, expr), type_to_db_name(ast_type_info(ast, expr)));

    log_excerpt_help(gutter, "You can make it explicit by changing it to:\n");
    switch (ast_type_info(ast, expr))
    {
        case TYPE_I64: /* fallthrough */
        case TYPE_U32: /* fallthrough */
        case TYPE_I32: /* fallthrough */
        case TYPE_U16: /* fallthrough */
        case TYPE_U8: log_excerpt(source, hl_int); break;

        case TYPE_F32: log_excerpt(source, hl_float); break;
        case TYPE_F64: log_excerpt(source, hl_double); break;
        case TYPE_STRING: log_excerpt(source, hl_string); break;

        case TYPE_INVALID:
        case TYPE_VOID:
        case TYPE_BOOL:
        case TYPE_ARRAY:
        case TYPE_LABEL:
        case TYPE_DABEL:
        case TYPE_ANY:
        case TYPE_USER_DEFINED_VAR_PTR: ODBUTIL_DEBUG_ASSERT(0, (void)0); break;
    }
}

void
warn_cast_implicit_conversion(
    const struct ast* ast,
    ast_id            cast,
    const char*       filename,
    const char*       source)
{
    ast_id    expr = ast->nodes[cast].cast.expr;
    enum type source_type = ast_type_info(ast, expr);
    enum type target_type = ast_type_info(ast, cast);

    log_flc_warn(
        filename,
        source,
        ast_loc(ast, cast),
        "Implicit conversion from {emph1:%s} to {emph2:%s} in expression\n",
        type_to_db_name(source_type),
        type_to_db_name(target_type));
    log_excerpt_2(
        source,
        ast_loc(ast, expr),
        ast_loc(ast, cast),
        type_to_db_name(source_type),
        type_to_db_name(target_type));
}

void
warn_cast_truncation(
    const struct ast* ast,
    ast_id            cast,
    const char*       filename,
    const char*       source)
{
    ast_id    expr = ast->nodes[cast].cast.expr;
    enum type source_type = ast_type_info(ast, expr);
    enum type target_type = ast_type_info(ast, cast);

    log_flc_warn(
        filename,
        source,
        ast_loc(ast, cast),
        "Value is truncated when converting from {emph1:%s} to {emph2:%s} in "
        "expression\n",
        type_to_db_name(source_type),
        type_to_db_name(target_type));
    log_excerpt_2(
        source,
        ast_loc(ast, expr),
        ast_loc(ast, cast),
        type_to_db_name(source_type),
        type_to_db_name(target_type));
}

void
warn_loop_for_default_step_may_be_incorrect(
    const struct ast* ast,
    ast_id            begin,
    ast_id            end,
    const char*       filename,
    const char*       source)
{
    int              gutter;
    struct utf8_span loc
        = utf8_span_union(ast_loc(ast, begin), ast_loc(ast, end));
    utf8_idx             ins = loc.off + loc.len;
    struct log_highlight hl_step_forwards[]
        = {{" STEP 1", "", {ins, 7}, LOG_INSERT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};
    struct log_highlight hl_step_backwards[]
        = {{" STEP -1", "", {ins, 8}, LOG_INSERT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};
    log_flc_warn(
        filename, source, loc, "For-loop direction may be incorrect.\n");
    gutter = log_excerpt_1(source, loc, "");
    log_excerpt_help(
        gutter,
        "If no STEP is specified, it will default to 1. You can silence this "
        "warning by making the STEP explicit:\n");
    log_excerpt(source, hl_step_forwards);
    log_excerpt(source, hl_step_backwards);
}

void
warn_loop_for_wrong_direction(
    const struct ast* ast,
    ast_id            begin,
    ast_id            end,
    ast_id            step,
    const char*       filename,
    const char*       source)
{
    struct utf8_span loc1
        = utf8_span_union(ast_loc(ast, begin), ast_loc(ast, end));
    struct utf8_span     loc2 = ast_loc(ast, step);
    struct log_highlight hl[]
        = {{"", "", loc1, LOG_HIGHLIGHT, LOG_MARKERS, 0},
           {"", "", loc2, LOG_HIGHLIGHT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};
    log_flc_warn(
        filename,
        source,
        loc1,
        "For-loop does nothing, because it STEPs in the wrong direction.\n");
    log_excerpt(source, hl);
}

void
warn_loop_for_wrong_direction_no_step(
    const struct ast* ast,
    ast_id            begin,
    ast_id            end,
    const char*       filename,
    const char*       source)
{
    int              gutter;
    struct utf8_span loc
        = utf8_span_union(ast_loc(ast, begin), ast_loc(ast, end));
    utf8_idx             ins = loc.off + loc.len;
    struct log_highlight hl[]
        = {{" STEP -1", "", {ins, 8}, LOG_INSERT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};
    log_flc_warn(
        filename,
        source,
        loc,
        "For-loop does nothing, because it STEPs in the wrong direction.\n");
    gutter = log_excerpt_1(source, loc, "");
    log_excerpt_help(
        gutter,
        "If no STEP is specified, it will default to 1. You can make a loop "
        "count backwards as follows:\n");
    log_excerpt(source, hl);
}

void
warn_loop_for_incorrect_next(
    struct ast* ast,
    ast_id      next,
    ast_id      loop_var,
    const char* filename,
    const char* source)
{
    int gutter;
    log_flc_warn(
        filename,
        source,
        ast_loc(ast, next),
        "Loop variable in next statement is different from the one used in the "
        "for-loop statement.\n");
    gutter = log_excerpt_1(source, ast_loc(ast, next), "");
    log_excerpt_note(gutter, "Loop variable declared here:\n");
    log_excerpt_1(source, ast_loc(ast, loop_var), "");
}

void
warn_initialization_implicit_conversion(
    const struct ast* ast, ast_id ass, const char* filename, const char* source)
{
    int    gutter;
    ast_id lhs = ast->nodes[ass].assignment.lvalue;
    ast_id rhs = ast->nodes[ass].assignment.expr;

    const char* as_type = type_to_db_name(ast_type_info(ast, rhs));
    char        ann[2] = {type_to_annotation(ast_type_info(ast, rhs)), '\0'};
    struct utf8_span lhs_loc = ast_loc(ast, lhs);
    utf8_idx         lhs_end = lhs_loc.off + lhs_loc.len;

    struct log_highlight hl_annotation[]
        = {{ann, "", {lhs_end, 1}, LOG_INSERT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};

    struct log_highlight hl_as_type[]
        = {{" AS ", "", {lhs_end, 4}, LOG_INSERT, {'^', '~', '~'}, 0},
           {as_type,
            "",
            {lhs_end, strlen(as_type)},
            LOG_INSERT,
            {'~', '~', '<'},
            0},
           LOG_HIGHLIGHT_SENTINAL};

    log_flc_warn(
        filename,
        source,
        ast_loc(ast, rhs),
        "Implicit conversion from {emph2:%s} to {emph1:%s} in variable "
        "initialization.\n",
        type_to_db_name(ast_type_info(ast, rhs)),
        type_to_db_name(ast_type_info(ast, lhs)));
    gutter = log_excerpt_binop(
        source,
        ast_loc(ast, lhs),
        ast->nodes[ass].assignment.op_location,
        ast_loc(ast, rhs),
        type_to_db_name(ast_type_info(ast, lhs)),
        type_to_db_name(ast_type_info(ast, rhs)));
    if (ann[0] != TA_NONE)
    {
        log_excerpt_help(gutter, "Annotate the variable:\n");
        log_excerpt(source, hl_annotation);
    }
    log_excerpt_help(
        gutter,
        "%sxplicitly declare the type of the variable:\n",
        ann[0] != TA_NONE ? "Or e" : "E");
    log_excerpt(source, hl_as_type);
}

void
warn_initialization_truncation(
    const struct ast* ast, ast_id ass, const char* filename, const char* source)
{
    int    gutter;
    ast_id lhs = ast->nodes[ass].assignment.lvalue;
    ast_id rhs = ast->nodes[ass].assignment.expr;

    const char* as_type = type_to_db_name(ast_type_info(ast, rhs));
    char        ann[2] = {type_to_annotation(ast_type_info(ast, rhs)), '\0'};
    struct utf8_span lhs_loc = ast_loc(ast, lhs);
    utf8_idx         lhs_end = lhs_loc.off + lhs_loc.len;

    struct log_highlight hl_annotation[]
        = {{ann, "", {lhs_end, 1}, LOG_INSERT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};

    struct log_highlight hl_as_type[]
        = {{" AS ", "", {lhs_end, 4}, LOG_INSERT, {'^', '~', '~'}, 0},
           {as_type,
            "",
            {lhs_end, strlen(as_type)},
            LOG_INSERT,
            {'~', '~', '<'},
            0},
           LOG_HIGHLIGHT_SENTINAL};

    log_flc_warn(
        filename,
        source,
        ast_loc(ast, rhs),
        "Value is truncated in conversion from {emph2:%s} to {emph1:%s} in "
        "variable initialization.\n",
        type_to_db_name(ast_type_info(ast, rhs)),
        type_to_db_name(ast_type_info(ast, lhs)));
    gutter = log_excerpt_binop(
        source,
        ast_loc(ast, lhs),
        ast->nodes[ass].assignment.op_location,
        ast_loc(ast, rhs),
        type_to_db_name(ast_type_info(ast, lhs)),
        type_to_db_name(ast_type_info(ast, rhs)));
    if (ann[0] != TA_NONE)
    {
        log_excerpt_help(gutter, "Annotate the variable:\n");
        log_excerpt(source, hl_annotation);
    }
    log_excerpt_help(
        gutter,
        "%sxplicitly declare the type of the variable:\n",
        ann[0] != TA_NONE ? "Or e" : "E");
    log_excerpt(source, hl_as_type);
}
