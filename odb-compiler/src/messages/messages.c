#include "odb-compiler/ast/ast.h"
#include "odb-compiler/messages/messages.h"
#include "odb-util/log.h"

static void
help_insert_explicit_cast(
    const char* source, struct utf8_span identifier_loc, enum type target_type)
{
    char                 ann[19] = " AS ";
    utf8_idx             ins = identifier_loc.off + identifier_loc.len;
    struct log_highlight hl[]
        = {{ann, "", {ins, 0}, LOG_INSERT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};

    ODBUTIL_DEBUG_ASSERT(strlen(type_to_db_name(target_type)) < 15, (void)0);
    hl[0].loc.len = strlen(strcat(ann, type_to_db_name(target_type)));

    log_help("Insert an explicit cast to silence this warning:\n");
    log_excerpt(source, hl);
}

int
err_assignment_incompatible_types(
    const struct ast* ast,
    ast_id            ass,
    ast_id            orig_decl,
    const char*       filename,
    const char*       source)
{
    ast_id lhs = ast->nodes[ass].assignment.lvalue;
    ast_id rhs = ast->nodes[ass].assignment.expr;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, ass) == AST_ASSIGNMENT,
        log_err("type: %d\n", ast_node_type(ast, ass)));

    log_flc(filename, source, ast_loc(ast, rhs));
    log_err(
        "Cannot assign {emph1:%s} to {emph0:%s}. Types are incompatible.\n",
        type_to_db_name(ast_type_info(ast, rhs)),
        type_to_db_name(ast_type_info(ast, lhs)));
    log_excerpt_binop(
        source,
        ast_loc(ast, lhs),
        ast->nodes[ass].assignment.op_location,
        ast_loc(ast, rhs),
        type_to_db_name(ast_type_info(ast, lhs)),
        type_to_db_name(ast_type_info(ast, rhs)));

    if (orig_decl > -1)
    {
        struct utf8_span orig_name;
        ODBUTIL_DEBUG_ASSERT(
            ast_node_type(ast, orig_decl) == AST_IDENTIFIER,
            log_err("type: %d\n", ast_node_type(ast, orig_decl)));
        orig_name = ast->nodes[orig_decl].identifier.name;

        log_flc(filename, source, ast_loc(ast, orig_decl));
        log_note(
            "{emph0:%.*s} was previously declared as {emph0:%s} here:\n",
            orig_name.len,
            source + orig_name.off,
            type_to_db_name(ast_type_info(ast, lhs)));
        log_excerpt_1(
            source,
            ast_loc(ast, orig_decl),
            type_to_db_name(ast_type_info(ast, lhs)),
            0);
    }

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

    log_flc(filename, source, ast_loc(ast, op));
    log_err(
        "Invalid conversion from {emph0:%s} to {emph1:%s} in binary "
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
    ast_id base = ast->nodes[op].binop.left;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, op) == AST_BINOP,
        log_err("type: %d\n", ast_node_type(ast, op)));
    ODBUTIL_DEBUG_ASSERT(
        ast->nodes[op].binop.op == BINOP_POW,
        log_err("op: %d\n", ast->nodes[op].binop.op));

    log_flc(filename, source, ast_loc(ast, base));
    log_err(
        "Incompatible base type {emph0:%s} can't be converted to {emph1:%s}.\n",
        type_to_db_name(base_type),
        type_to_db_name(target_type));
    log_excerpt_2(
        source,
        ast_loc(ast, base),
        ast->nodes[op].binop.op_location,
        type_to_db_name(base_type),
        "",
        0,
        1);
    log_note(
        "The base can be a {emph1:%s} or {emph1:%s}.\n",
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
    ast_id base = ast->nodes[op].binop.left;
    ast_id exp = ast->nodes[op].binop.right;

    log_flc(filename, source, ast_loc(ast, exp));
    log_err(
        "Incompatible exponent type {emph0:%s} can't be converted to "
        "{emph1:%s}.\n",
        type_to_db_name(exp_type),
        type_to_db_name(target_type));
    log_excerpt_binop(
        source,
        ast_loc(ast, base),
        ast->nodes[op].binop.op_location,
        ast_loc(ast, exp),
        "",
        type_to_db_name(exp_type));
    log_help(
        "The exponent can be an {emph1:%s}, {emph1:%s} or {emph1:%s}.\n",
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
    log_flc(filename, source, ast_loc(ast, expr));
    log_err(
        "Cannot evaluate {emph0:%s} as a boolean expression.\n",
        type_to_db_name(ast_type_info(ast, expr)));
    log_excerpt_1(
        source,
        ast_loc(ast, expr),
        type_to_db_name(ast_type_info(ast, expr)),
        0);

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
        log_err("type: %d\n", ast_node_type(ast, cast)));

    log_flc(filename, source, ast_loc(ast, cast));
    log_err(
        "Cannot cast from {emph0:%s} to {emph1:%s}: Types are incompatible\n",
        type_to_db_name(source_type),
        type_to_db_name(target_type));
    log_excerpt_2(
        source,
        ast_loc(ast, expr),
        ast_loc(ast, cast),
        type_to_db_name(source_type),
        type_to_db_name(target_type),
        0,
        1);

    return -1;
}

int
err_func_call_incompatible_types(
    const struct ast* ast,
    ast_id            arg,
    ast_id            param,
    int               arg_num,
    const char*       filename,
    const char*       source)
{
    ast_id           identifier;
    struct utf8_span param_type_loc;

    ODBUTIL_DEBUG_ASSERT(arg > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(param > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, param) == AST_PARAM,
        log_err("type: %d\n", ast_node_type(ast, param)));

    identifier = ast->nodes[param].param.identifier;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, identifier)));

    param_type_loc = ast->nodes[param].param.as > -1
                         ? ast_loc(ast, ast->nodes[param].param.as)
                         : ast_loc(ast, param);

    log_flc(filename, source, ast_loc(ast, arg));
    log_err(
        "Cannot convert %d%s argument from {emph0:%s} to {emph1:%s} in "
        "function call. Types are incompatible.\n",
        arg_num,
        arg_num == 1   ? "st"
        : arg_num == 2 ? "nd"
        : arg_num == 3 ? "rd"
                       : "th",
        type_to_db_name(ast_type_info(ast, arg)),
        type_to_db_name(ast_type_info(ast, param)));
    log_excerpt_1(
        source, ast_loc(ast, arg), type_to_db_name(ast_type_info(ast, arg)), 1);

    log_flc(filename, source, param_type_loc);
    log_note("Function parameter was declared here:\n");
    log_excerpt_1(source, param_type_loc, "", 0);

    return -1;
}

int
err_func_redefinition(
    const struct ast* func_ast,
    ast_id            func,
    const char*       filename,
    const char*       source,
    const struct ast* prev_ast,
    ast_id            prev_func,
    const char*       prev_filename,
    const char*       prev_source)
{
    ast_id           identifier, prev_identifier;
    struct utf8_span name;
    struct utf8_span loc, prev_loc;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(func_ast, func) == AST_FUNC1,
        log_err("type: %d\n", ast_node_type(func_ast, func)));
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(prev_ast, prev_func) == AST_FUNC1,
        log_err("type: %d\n", ast_node_type(prev_ast, prev_func)));

    identifier = func_ast->nodes[func].func1.identifier;
    prev_identifier = prev_ast->nodes[prev_func].func1.identifier;
    name = func_ast->nodes[identifier].identifier.name;
    loc = ast_loc(func_ast, identifier);
    prev_loc = ast_loc(prev_ast, prev_identifier);

    log_flc(filename, source, loc);
    log_err(
        "Redefinition of function {quote:%.*s}.\n",
        name.len,
        source + name.off);
    log_excerpt_1(source, loc, "", 0);

    log_flc(prev_filename, prev_source, prev_loc);
    log_note("Previously defined here:\n");
    log_excerpt_1(prev_source, prev_loc, "", 0);

    return -1;
}

int
err_func_return_incompatible_types(
    const struct ast* ast,
    ast_id            func,
    ast_id            retval,
    const char*       filename,
    const char*       source)
{
    ast_id           f2, identifier;
    struct utf8_span ret_type_loc;

    ODBUTIL_DEBUG_ASSERT(retval > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, func) == AST_FUNC1,
        log_err("type: %d\n", ast_node_type(ast, func)));

    identifier = ast->nodes[func].func1.identifier;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, identifier)));

    f2 = ast->nodes[func].func1.func2;
    ret_type_loc = ast->nodes[f2].func2.as > -1
                       ? ast_loc(ast, ast->nodes[f2].func2.as)
                       : ast_loc(ast, identifier);

    log_flc(filename, source, ast_loc(ast, retval));
    log_err(
        "Cannot convert {emph0:%s} to {emph1:%s} in function return. Types are "
        "incompatible.\n",
        type_to_db_name(ast_type_info(ast, retval)),
        type_to_db_name(ast_type_info(ast, identifier)));
    log_excerpt_1(
        source,
        ast_loc(ast, retval),
        type_to_db_name(ast_type_info(ast, retval)),
        0);

    log_flc(filename, source, ret_type_loc);
    log_note("Function return type was declared here:\n");
    log_excerpt_1(source, ret_type_loc, "", 1);

    return -1;
}

int
err_func_missing_return_value(
    const struct ast* ast,
    ast_id            func,
    struct utf8_span  ret_loc,
    const char*       filename,
    const char*       source)
{
    ast_id           f2, identifier;
    struct utf8_span ret_type_loc;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, func) == AST_FUNC1,
        log_err("type: %d\n", ast_node_type(ast, func)));

    identifier = ast->nodes[func].func1.identifier;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, identifier)));

    f2 = ast->nodes[func].func1.func2;
    ret_type_loc = ast->nodes[f2].func2.as > -1
                       ? ast_loc(ast, ast->nodes[f2].func2.as)
                       : ast_loc(ast, identifier);

    log_flc(filename, source, ret_loc);
    log_err("Missing return value.\n");
    log_excerpt_1(source, ret_loc, "", 0);

    log_flc(filename, source, ret_type_loc);
    log_note("Function return type was declared here:\n");
    log_excerpt_1(source, ret_type_loc, "", 0);

    return -1;
}

int
err_loop_duplicate_name(
    const struct ast* ast,
    struct utf8_span  inner_name,
    struct utf8_span  outer_name,
    const char*       filename,
    const char*       source)
{
    log_flc(filename, source, inner_name);
    log_err("Loop name already in use.\n");
    log_excerpt_1(source, inner_name, "", 0);

    log_flc(filename, source, outer_name);
    log_note("Previously defined here:\n");
    log_excerpt_1(source, outer_name, "", 0);

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
    if (first_loop == -1)
    {
        log_flc(filename, source, ast_loc(ast, cont));
        log_err("CONTINUE statement must be inside a loop.\n");
        log_excerpt_1(source, ast_loc(ast, cont), "", 0);
    }
    else
    {
        struct utf8_span name = ast->nodes[first_loop].loop1.name.len
                                    ? ast->nodes[first_loop].loop1.name
                                : ast->nodes[first_loop].loop1.implicit_name.len
                                    ? ast->nodes[first_loop].loop1.implicit_name
                                    : empty_utf8_span();
        log_flc(filename, source, ast->nodes[cont].cont.name);
        log_err("Unknown loop name referenced in CONTINUE statement.\n");
        log_excerpt_1(source, ast->nodes[cont].cont.name, "", 0);

        if (name.len)
        {
            log_flc(filename, source, name);
            log_help(
                "Did you mean {quote:%.*s}?\n", name.len, source + name.off);
            log_excerpt_1(source, name, "", 0);
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
    log_flc(filename, source, ast_loc(ast, exit));
    log_err("EXIT statement must be inside a loop.\n");
    log_excerpt_1(source, ast_loc(ast, exit), "", 0);

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
    struct utf8_span name;

    if (ast->nodes[first_loop].loop1.name.len > 0)
        name = ast->nodes[first_loop].loop1.name;
    else if (ast->nodes[first_loop].loop1.implicit_name.len > 0)
        name = ast->nodes[first_loop].loop1.implicit_name;
    else
        name = empty_utf8_span();

    log_flc(filename, source, ast->nodes[exit].loop_exit.name);
    log_err("Unknown loop name referenced in EXIT statement.\n");
    log_excerpt_1(source, ast->nodes[exit].loop_exit.name, "", 0);

    if (name.len)
    {
        log_flc(filename, source, name);
        log_help("Did you mean {quote:%.*s}?\n", name.len, source + name.off);
        log_excerpt_1(source, name, "", 0);
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
    struct utf8_span loc1
        = utf8_span_union(ast_loc(ast, begin), ast_loc(ast, end));
    struct utf8_span     loc2 = ast_loc(ast, step);
    struct log_highlight hl[]
        = {{"", "", loc1, LOG_HIGHLIGHT, LOG_MARKERS, 0},
           {"", "", loc2, LOG_HIGHLIGHT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};
    log_flc(filename, source, loc1);
    log_err("Unable to determine direction of for-loop.\n");
    log_excerpt(source, hl);

    log_note(
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
    log_flc(filename, source, location);
    log_err("Unterminated remark.\n");
    log_excerpt_1(source, location, "Remark starts here.", 0);

    return -1;
}

int
err_var_decl_init_incompatible_types(
    const struct ast* ast,
    ast_id            var_decl,
    const char*       filename,
    const char*       source)
{
    ast_id init_expr;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, var_decl),
        log_err("type: %d\n", ast_node_type(ast, var_decl)));

    init_expr = ast->nodes[var_decl].var_decl1.init_expr;
    ODBUTIL_DEBUG_ASSERT(init_expr > -1, (void)0);

    log_flc(filename, source, ast_loc(ast, init_expr));
    log_err(
        "Cannot initialize {emph0:%s} with a {emph1:%s}. Types are "
        "incompatible.\n",
        type_to_db_name(ast_type_info(ast, var_decl)),
        type_to_db_name(ast_type_info(ast, init_expr)));
    log_excerpt_2(
        source,
        ast_loc(ast, var_decl),
        ast_loc(ast, init_expr),
        type_to_db_name(ast_type_info(ast, var_decl)),
        type_to_db_name(ast_type_info(ast, init_expr)),
        0,
        1);

    return -1;
}

int
err_var_decl_redeclaration(
    const struct ast* ast,
    ast_id            identifier,
    const char*       filename,
    const char*       source,
    const struct ast* prev_ast,
    ast_id            prev_identifier,
    const char*       prev_filename,
    const char*       prev_source)
{
    struct utf8_span name;
    struct utf8_span loc, prev_loc;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, identifier)));
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(prev_ast, prev_identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(prev_ast, prev_identifier)));

    name = ast->nodes[identifier].identifier.name;
    loc = ast_loc(ast, identifier);
    prev_loc = ast_loc(prev_ast, prev_identifier);

    log_flc(filename, source, loc);
    log_err(
        "Variable {quote:%.*s} already exists.\n", name.len, source + name.off);
    log_excerpt_1(source, loc, "", 0);

    log_flc(prev_filename, prev_source, prev_loc);
    log_note("Previously defined here:\n");
    log_excerpt_1(prev_source, prev_loc, "", 0);

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
    ast_id lhs = ast->nodes[ass].assignment.lvalue;
    ast_id rhs = ast->nodes[ass].assignment.expr;

    log_flc(filename, source, ast_loc(ast, rhs));
    log_warn(
        "Implicit conversion from {emph1:%s} to {emph0:%s} in assignment.\n",
        type_to_db_name(ast_type_info(ast, rhs)),
        type_to_db_name(ast_type_info(ast, lhs)));
    log_excerpt_binop(
        source,
        ast_loc(ast, lhs),
        ast->nodes[ass].assignment.op_location,
        ast_loc(ast, rhs),
        type_to_db_name(ast_type_info(ast, lhs)),
        type_to_db_name(ast_type_info(ast, rhs)));

    if (orig_decl > -1)
    {
        struct utf8_span orig_name;
        ODBUTIL_DEBUG_ASSERT(
            ast_node_type(ast, orig_decl) == AST_IDENTIFIER,
            log_err("type: %d\n", ast_node_type(ast, orig_decl)));
        orig_name = ast->nodes[orig_decl].identifier.name;

        log_flc(filename, source, ast_loc(ast, orig_decl));
        log_note(
            "{emph0:%.*s} was previously declared as {emph0:%s} here:\n",
            orig_name.len,
            source + orig_name.off,
            type_to_db_name(ast_type_info(ast, lhs)));
        log_excerpt_1(
            source,
            ast_loc(ast, orig_decl),
            type_to_db_name(ast_type_info(ast, lhs)),
            0);
    }

    help_insert_explicit_cast(
        source, ast_loc(ast, rhs), ast_type_info(ast, lhs));
}

void
warn_assignment_truncation(
    const struct ast* ast,
    ast_id            ass,
    ast_id            orig_decl,
    const char*       filename,
    const char*       source)
{
    ast_id lhs = ast->nodes[ass].assignment.lvalue;
    ast_id rhs = ast->nodes[ass].assignment.expr;

    log_flc(filename, source, ast_loc(ast, rhs));
    log_warn(
        "Value is truncated in conversion from {emph1:%s} to {emph0:%s} in "
        "assignment.\n",
        type_to_db_name(ast_type_info(ast, rhs)),
        type_to_db_name(ast_type_info(ast, lhs)));
    log_excerpt_binop(
        source,
        ast_loc(ast, lhs),
        ast->nodes[ass].assignment.op_location,
        ast_loc(ast, rhs),
        type_to_db_name(ast_type_info(ast, lhs)),
        type_to_db_name(ast_type_info(ast, rhs)));

    if (orig_decl > -1)
    {
        struct utf8_span orig_name;
        ODBUTIL_DEBUG_ASSERT(
            ast_node_type(ast, orig_decl) == AST_IDENTIFIER,
            log_err("type: %d\n", ast_node_type(ast, orig_decl)));
        orig_name = ast->nodes[orig_decl].identifier.name;

        log_flc(filename, source, ast_loc(ast, orig_decl));
        log_note(
            "{emph0:%.*s} was previously declared as {emph0:%s} here:\n",
            orig_name.len,
            source + orig_name.off,
            type_to_db_name(ast_type_info(ast, lhs)));
        log_excerpt_1(
            source,
            ast_loc(ast, orig_decl),
            type_to_db_name(ast_type_info(ast, lhs)),
            0);
    }

    help_insert_explicit_cast(
        source, ast_loc(ast, rhs), ast_type_info(ast, lhs));
}

void
warn_binop_implicit_conversion(
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

    log_flc(filename, source, ast_loc(ast, op));
    log_warn(
        "Implicit conversion from {emph0:%s} to {emph1:%s} in binary "
        "expression.\n",
        type_to_db_name(source_type),
        type_to_db_name(target_type));
    log_excerpt_binop(
        source,
        ast_loc(ast, lhs),
        ast->nodes[op].binop.op_location,
        ast_loc(ast, rhs),
        type_to_db_name(lhs == source_node ? source_type : target_type),
        type_to_db_name(lhs == source_node ? target_type : source_type));
    help_insert_explicit_cast(source, ast_loc(ast, source_node), target_type);
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

    log_flc(filename, source, ast_loc(ast, op));
    log_warn(
        "Value is truncated when converting from {emph0:%s} to {emph1:%s} in "
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
    ast_id base = ast->nodes[op].binop.left;

    log_flc(filename, source, ast_loc(ast, base));
    log_warn(
        "Implicit conversion of base from {emph0:%s} to {emph1:%s}.\n",
        type_to_db_name(base_type),
        type_to_db_name(target_type));
    log_excerpt_2(
        source,
        ast_loc(ast, base),
        ast->nodes[op].binop.op_location,
        type_to_db_name(base_type),
        "",
        0,
        1);
    log_note(
        "The base can be a {emph1:%s} or {emph1:%s}\n",
        type_to_db_name(TYPE_F32),
        type_to_db_name(TYPE_F64));
    help_insert_explicit_cast(source, ast_loc(ast, base), target_type);
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
    ast_id base = ast->nodes[op].binop.left;
    ast_id exp = ast->nodes[op].binop.right;

    log_flc(filename, source, ast_loc(ast, base));
    log_warn(
        "Base value is truncated when converting from {emph0:%s} to {emph1:%s} "
        "in binary expression.\n",
        type_to_db_name(base_type),
        type_to_db_name(target_type));
    log_excerpt_binop(
        source,
        ast_loc(ast, base),
        ast->nodes[op].binop.op_location,
        ast_loc(ast, exp),
        type_to_db_name(base_type),
        type_to_db_name(ast_type_info(ast, exp)));
    log_note(
        "The base can be a {emph1:%s} or {emph1:%s}.\n",
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
    ast_id base = ast->nodes[op].binop.left;
    ast_id exp = ast->nodes[op].binop.right;

    log_flc(filename, source, ast_loc(ast, exp));
    log_warn(
        "Implicit conversion of exponent from {emph1:%s} to {emph0:%s}.\n",
        type_to_db_name(exp_type),
        type_to_db_name(target_type));
    log_excerpt_binop(
        source,
        ast_loc(ast, base),
        ast->nodes[op].binop.op_location,
        ast_loc(ast, exp),
        "",
        type_to_db_name(exp_type));

    if (target_type == TYPE_F32)
        log_help(
            "The exponent needs to be the same type as the base when working "
            "with floating point types.\n");

    if (exp_type == TYPE_I64 || exp_type == TYPE_U32)
        log_help(
            "{emph1:INTEGER} is the largest possible integral type for "
            "exponents.\n");

    log_help(
        "The exponent can be an {emph1:%s}, {emph1:%s} or {emph1:%s}.\n",
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
    ast_id base = ast->nodes[op].binop.left;
    ast_id exp = ast->nodes[op].binop.right;

    log_flc(filename, source, ast_loc(ast, exp));
    log_warn(
        "Exponent value is truncated when converting from {emph1:%s} to "
        "{emph0:%s}.\n",
        type_to_db_name(exp_type),
        type_to_db_name(target_type));
    log_excerpt_binop(
        source,
        ast_loc(ast, base),
        ast->nodes[op].binop.op_location,
        ast_loc(ast, exp),
        target_type == TYPE_F32 ? type_to_db_name(TYPE_F32) : "",
        type_to_db_name(exp_type));

    if (target_type == TYPE_F32)
        log_help(
            "The exponent is always converted to the same type as the base "
            "when using floating point exponents.\n");

    if (target_type == TYPE_I32)
        log_help(
            "{emph1:INTEGER} is the largest possible integral type for "
            "exponents.\n");

    log_help(
        "The exponent can be an {emph1:%s}, {emph1:%s} or {emph1:%s}.\n",
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
    log_flc(filename, source, ast_loc(ast, expr));
    log_warn(
        "Implicit evaluation of {emph0:%s} as a boolean expression.\n",
        type_to_db_name(ast_type_info(ast, expr)));
    log_excerpt_1(
        source,
        ast_loc(ast, expr),
        type_to_db_name(ast_type_info(ast, expr)),
        0);

    log_help("You can make it explicit by changing it to:\n");
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
warn_func_call_implicit_conversion(
    const struct ast* ast,
    ast_id            arg,
    ast_id            param,
    int               arg_num,
    const char*       filename,
    const char*       source)
{
    ast_id           identifier;
    struct utf8_span param_type_loc;

    ODBUTIL_DEBUG_ASSERT(arg > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(param > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, param) == AST_PARAM,
        log_err("type: %d\n", ast_node_type(ast, param)));

    identifier = ast->nodes[param].param.identifier;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, identifier)));

    param_type_loc = ast->nodes[param].param.as > -1
                         ? ast_loc(ast, ast->nodes[param].param.as)
                         : ast_loc(ast, param);

    log_flc(filename, source, ast_loc(ast, arg));
    log_warn(
        "Implicit conversion of %d%s argument from {emph0:%s} to {emph1:%s} in "
        "function call.\n",
        arg_num,
        arg_num == 1   ? "st"
        : arg_num == 2 ? "nd"
        : arg_num == 3 ? "rd"
                       : "th",
        type_to_db_name(ast_type_info(ast, arg)),
        type_to_db_name(ast_type_info(ast, param)));
    log_excerpt_1(
        source, ast_loc(ast, arg), type_to_db_name(ast_type_info(ast, arg)), 1);

    log_flc(filename, source, param_type_loc);
    log_note("Function parameter type is declared here:\n");
    log_excerpt_1(source, param_type_loc, "", 0);

    help_insert_explicit_cast(
        source, ast_loc(ast, arg), ast_type_info(ast, param));
}

void
warn_func_call_truncation(
    const struct ast* ast,
    ast_id            arg,
    ast_id            param,
    int               arg_num,
    const char*       filename,
    const char*       source)
{
    ast_id           identifier;
    struct utf8_span param_type_loc;

    ODBUTIL_DEBUG_ASSERT(arg > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(param > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, param) == AST_PARAM,
        log_err("type: %d\n", ast_node_type(ast, param)));

    identifier = ast->nodes[param].param.identifier;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, identifier)));

    param_type_loc = ast->nodes[param].param.as > -1
                         ? ast_loc(ast, ast->nodes[param].param.as)
                         : ast_loc(ast, param);

    log_flc(filename, source, ast_loc(ast, arg));
    log_warn(
        "Value is truncated when converting %d%s argument from {emph0:%s} to "
        "{emph1:%s} in function call.\n",
        arg_num,
        arg_num == 1   ? "st"
        : arg_num == 2 ? "nd"
        : arg_num == 3 ? "rd"
                       : "th",
        type_to_db_name(ast_type_info(ast, param)),
        type_to_db_name(ast_type_info(ast, arg)));
    log_excerpt_1(
        source, ast_loc(ast, arg), type_to_db_name(ast_type_info(ast, arg)), 1);

    log_flc(filename, source, param_type_loc);
    log_note("Function return type was declared here:\n");
    log_excerpt_1(source, param_type_loc, "", 0);
}

void
warn_func_return_implicit_conversion(
    const struct ast* ast,
    ast_id            func,
    ast_id            retval,
    const char*       filename,
    const char*       source)
{
    ast_id           f2, identifier;
    struct utf8_span ret_type_loc;

    ODBUTIL_DEBUG_ASSERT(retval > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, func) == AST_FUNC1,
        log_err("type: %d\n", ast_node_type(ast, func)));

    identifier = ast->nodes[func].func1.identifier;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, identifier)));

    f2 = ast->nodes[func].func1.func2;
    ret_type_loc = ast->nodes[f2].func2.as > -1
                       ? ast_loc(ast, ast->nodes[f2].func2.as)
                       : ast_loc(ast, identifier);

    log_flc(filename, source, ast_loc(ast, retval));
    log_warn(
        "Implicit conversion from {emph0:%s} to {emph1:%s} in function "
        "return.\n",
        type_to_db_name(ast_type_info(ast, retval)),
        type_to_db_name(ast_type_info(ast, identifier)));
    log_excerpt_1(
        source,
        ast_loc(ast, retval),
        type_to_db_name(ast_type_info(ast, retval)),
        0);

    log_flc(filename, source, ret_type_loc);
    log_note("Function return type was declared here:\n");
    log_excerpt_1(source, ret_type_loc, "", 1);
    help_insert_explicit_cast(
        source, ast_loc(ast, retval), ast_type_info(ast, identifier));
}

void
warn_func_return_truncation(
    const struct ast* ast,
    ast_id            func,
    ast_id            retval,
    const char*       filename,
    const char*       source)
{
    ast_id           f2, identifier;
    struct utf8_span ret_type_loc;

    ODBUTIL_DEBUG_ASSERT(retval > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, func) == AST_FUNC1,
        log_err("type: %d\n", ast_node_type(ast, func)));

    identifier = ast->nodes[func].func1.identifier;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, identifier) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, identifier)));

    f2 = ast->nodes[func].func1.func2;
    ret_type_loc = ast->nodes[f2].func2.as > -1
                       ? ast_loc(ast, ast->nodes[f2].func2.as)
                       : ast_loc(ast, identifier);

    log_flc(filename, source, ast_loc(ast, retval));
    log_warn(
        "Value is truncated when converting from {emph1:%s} to {emph0:%s} in "
        "function return.\n",
        type_to_db_name(ast_type_info(ast, retval)),
        type_to_db_name(ast_type_info(ast, identifier)));
    log_excerpt_1(
        source,
        ast_loc(ast, retval),
        type_to_db_name(ast_type_info(ast, retval)),
        0);

    log_flc(filename, source, ret_type_loc);
    log_note("Function return type was declared here:\n");
    log_excerpt_1(source, ret_type_loc, "", 1);
    help_insert_explicit_cast(
        source, ast_loc(ast, retval), ast_type_info(ast, identifier));
}

void
warn_loop_exit_ambiguous_name(
    const struct ast* ast,
    ast_id            exit,
    struct utf8_span  name,
    struct utf8_span  outer_name,
    const char*       filename,
    const char*       source)
{
    struct utf8_span exit_loc;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, exit) == AST_LOOP_EXIT,
        log_err("type: %d\n", ast_node_type(ast, exit)));
    exit_loc = ast->nodes[exit].loop_exit.name;

    log_flc(filename, source, exit_loc);
    log_warn("There are two nested loops sharing the same name.\n");
    log_excerpt_1(source, exit_loc, "", 0);

    log_flc(filename, source, name);
    log_note("This exit statement will exit the inner loop:\n");
    log_excerpt_1(source, name, "", 0);

    log_flc(filename, source, outer_name);
    log_note("Outer loop with the same name defined here:\n");
    log_excerpt_1(source, outer_name, "", 0);
}

void
warn_loop_for_default_step_may_be_incorrect(
    const struct ast* ast,
    ast_id            begin,
    ast_id            end,
    const char*       filename,
    const char*       source)
{
    struct utf8_span loc
        = utf8_span_union(ast_loc(ast, begin), ast_loc(ast, end));
    utf8_idx             ins = loc.off + loc.len;
    struct log_highlight hl_step_forwards[]
        = {{" STEP 1", "", {ins, 7}, LOG_INSERT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};
    struct log_highlight hl_step_backwards[]
        = {{" STEP -1", "", {ins, 8}, LOG_INSERT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};
    log_flc(filename, source, loc);
    log_warn("For-loop direction may be incorrect.\n");
    log_excerpt_1(source, loc, "", 0);

    log_help(
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
    log_flc(filename, source, loc1);
    log_warn(
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
    struct utf8_span loc
        = utf8_span_union(ast_loc(ast, begin), ast_loc(ast, end));
    utf8_idx             ins = loc.off + loc.len;
    struct log_highlight hl[]
        = {{" STEP -1", "", {ins, 8}, LOG_INSERT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};
    log_flc(filename, source, loc);
    log_warn(
        "For-loop does nothing, because it STEPs in the wrong direction.\n");
    log_excerpt_1(source, loc, "", 0);
    log_help(
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
    log_flc(filename, source, ast_loc(ast, next));
    log_warn(
        "Loop variable in next statement is different from the one used in the "
        "for-loop statement.\n");
    log_excerpt_1(source, ast_loc(ast, next), "", 0);

    log_flc(filename, source, ast_loc(ast, loop_var));
    log_note("Loop variable declared here:\n");
    log_excerpt_1(source, ast_loc(ast, loop_var), "", 0);
}

void
warn_var_decl_implicit_conversion(
    const struct ast* ast,
    ast_id            var_decl,
    const char*       filename,
    const char*       source)
{
    ast_id           decl2 = ast->nodes[var_decl].var_decl1.var_decl2;
    ast_id           lhs = ast->nodes[decl2].var_decl2.identifier;
    ast_id           rhs = ast->nodes[var_decl].var_decl1.init_expr;
    struct utf8_span op_loc = ast->nodes[decl2].var_decl2.op_location;
    enum type        lhs_type = ast_type_info(ast, lhs);
    enum type        rhs_type = ast_type_info(ast, rhs);
    const char*      lhs_type_name = type_to_db_name(lhs_type);
    const char*      rhs_type_name = type_to_db_name(rhs_type);

    char             ann[2] = {type_to_annotation(rhs_type), '\0'};
    struct utf8_span lhs_loc = ast_loc(ast, lhs);
    utf8_idx         lhs_end = lhs_loc.off + lhs_loc.len;

    /* clang-format off */
    struct log_highlight hl_annotation[] =
        {{ann, "", {lhs_end, 1}, LOG_INSERT, LOG_MARKERS, 0},
        LOG_HIGHLIGHT_SENTINAL
    };
    struct log_highlight hl_as_type[] = {
        {" AS ", "", {lhs_end, 4}, LOG_INSERT, {'^', '~', '~'}, 0},
        {rhs_type_name, "", {lhs_end, strlen(rhs_type_name)}, LOG_INSERT, {'~', '~', '<'}, 0},
        LOG_HIGHLIGHT_SENTINAL
    };
    /* clang-format on */

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, var_decl) == AST_VAR_DECL1,
        log_err("type: %d\n", ast_node_type(ast, var_decl)));

    log_flc(filename, source, ast_loc(ast, rhs));
    log_warn(
        "Implicit conversion from {emph1:%s} to {emph0:%s} in variable "
        "initialization.\n",
        rhs_type_name,
        lhs_type_name);
    log_excerpt_binop(
        source,
        ast_loc(ast, lhs),
        op_loc,
        ast_loc(ast, rhs),
        lhs_type_name,
        rhs_type_name);

    if (ann[0] != TA_NONE)
    {
        log_help("Annotate the variable:\n");
        log_excerpt(source, hl_annotation);
    }
    log_help(
        "%sxplicitly declare the type of the variable:\n",
        ann[0] != TA_NONE ? "Or e" : "E");
    log_excerpt(source, hl_as_type);
}

void
warn_var_decl_truncation(
    const struct ast* ast,
    ast_id            var_decl,
    const char*       filename,
    const char*       source)
{
    ast_id           decl2 = ast->nodes[var_decl].var_decl1.var_decl2;
    ast_id           lhs = ast->nodes[decl2].var_decl2.identifier;
    ast_id           rhs = ast->nodes[var_decl].var_decl1.init_expr;
    struct utf8_span op_loc = ast->nodes[decl2].var_decl2.op_location;
    enum type        lhs_type = ast_type_info(ast, lhs);
    enum type        rhs_type = ast_type_info(ast, rhs);
    const char*      lhs_type_name = type_to_db_name(lhs_type);
    const char*      rhs_type_name = type_to_db_name(rhs_type);

    char             ann[2] = {type_to_annotation(rhs_type), '\0'};
    struct utf8_span lhs_loc = ast_loc(ast, lhs);
    utf8_idx         lhs_end = lhs_loc.off + lhs_loc.len;

    /* clang-format off */
    struct log_highlight hl_annotation[] =
        {{ann, "", {lhs_end, 1}, LOG_INSERT, LOG_MARKERS, 0},
        LOG_HIGHLIGHT_SENTINAL
    };
    struct log_highlight hl_as_type[] = {
        {" AS ", "", {lhs_end, 4}, LOG_INSERT, {'^', '~', '~'}, 0},
        {rhs_type_name, "", {lhs_end, strlen(rhs_type_name)}, LOG_INSERT, {'~', '~', '<'}, 0},
        LOG_HIGHLIGHT_SENTINAL
    };
    /* clang-format on */

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, var_decl) == AST_VAR_DECL1,
        log_err("type: %d\n", ast_node_type(ast, var_decl)));

    log_flc(filename, source, ast_loc(ast, rhs));
    log_warn(
        "Value is truncated in conversion from {emph1:%s} to {emph0:%s} in "
        "variable initialization.\n",
        rhs_type_name,
        lhs_type_name);
    log_excerpt_binop(
        source,
        ast_loc(ast, lhs),
        op_loc,
        ast_loc(ast, rhs),
        type_to_db_name(ast_type_info(ast, lhs)),
        type_to_db_name(ast_type_info(ast, rhs)));

    if (ann[0] != TA_NONE)
    {
        log_help("Annotate the variable:\n");
        log_excerpt(source, hl_annotation);
    }
    log_help(
        "%sxplicitly declare the type of the variable:\n",
        ann[0] != TA_NONE ? "Or e" : "E");
    log_excerpt(source, hl_as_type);
}
