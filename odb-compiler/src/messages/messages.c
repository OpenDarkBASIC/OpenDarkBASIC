#include "odb-compiler/ast/ast.h"
#include "odb-compiler/messages/messages.h"
#include "odb-util/log.h"

static void
help_insert_explicit_cast(
    const char*         source,
    struct utf8_span    identifier_loc,
    enum primitive_type target_type)
{
    struct utf8_view ins1 = cstr_utf8_view(" AS ");
    struct utf8_view ins2 = cstr_utf8_view(primitive_type_name(target_type));
    struct utf8_view ann = empty_utf8_view();
    utf8_idx         loc_end = identifier_loc.off + identifier_loc.len;
    struct log_highlight hl[]
        = {{ins1, ann, {loc_end, ins1.len}, LOG_INSERT, "^~~", 0},
           {ins2, ann, {loc_end, ins2.len}, LOG_INSERT, "~~<", 0},
           LOG_HIGHLIGHT_SENTINAL};

    log_help("Insert an explicit cast to silence this warning:\n");
    log_excerpt(source, hl);
}

int
err_assignment_incompatible_types(
    const struct ast* ast,
    ast_id            ass,
    ast_id            first_occurrence,
    const char*       filename,
    const char*       source)
{
    ast_id           lhs = ast->nodes[ass].assignment.lvalue;
    ast_id           rhs = ast->nodes[ass].assignment.expr;
    struct utf8_view lhs_tname
        = type_name(ast_type_info(ast, lhs), ast, source);
    struct utf8_view rhs_tname
        = type_name(ast_type_info(ast, rhs), ast, source);

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, ass) == AST_ASSIGNMENT,
        log_err("type: %d\n", ast_node_type(ast, ass)));
    ODBUTIL_DEBUG_ASSERT(
        first_occurrence == -1
            || ast_node_type(ast, first_occurrence) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, first_occurrence)));

    log_flc(filename, source, ast_loc(ast, rhs));
    log_err(
        "Cannot assign {emph1:%.*s} to {emph0:%.*s}. Types are incompatible.\n",
        rhs_tname.len,
        rhs_tname.data + rhs_tname.off,
        lhs_tname.len,
        lhs_tname.data + lhs_tname.off);
    log_excerpt_binop(
        source,
        ast_loc(ast, lhs),
        ast->nodes[ass].assignment.op_location,
        ast_loc(ast, rhs),
        lhs_tname,
        rhs_tname);

    if (first_occurrence > -1)
    {
        struct utf8_span first_name
            = ast->nodes[first_occurrence].identifier.name;
        log_flc(filename, source, first_name);
        log_note(
            "{emph0:%.*s} was previously declared as {emph0:%.*s} here:\n",
            first_name.len,
            source + first_name.off,
            lhs_tname.len,
            lhs_tname.data + lhs_tname.off);
        log_excerpt_1(source, first_name, lhs_tname, 0);
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
    ast_id           lhs = ast->nodes[op].binop.left;
    ast_id           rhs = ast->nodes[op].binop.right;
    union type       source_type = ast_type_info(ast, source_node);
    union type       target_type = ast_type_info(ast, op);
    struct utf8_view source_tname = type_name(source_type, ast, source);
    struct utf8_view target_tname = type_name(target_type, ast, source);

    log_flc(filename, source, ast_loc(ast, op));
    log_err(
        "Invalid conversion from {emph0:%.*s} to {emph1:%.*s} in binary "
        "expression. Types are incompatible.\n",
        source_tname.len,
        source_tname.data + source_tname.off,
        target_tname.len,
        target_tname.data + target_tname.off);
    log_excerpt_binop(
        source,
        ast_loc(ast, lhs),
        ast->nodes[op].binop.op_location,
        ast_loc(ast, rhs),
        lhs == source_node ? source_tname : target_tname,
        lhs == source_node ? target_tname : source_tname);

    return -1;
}

int
err_binop_pow_incompatible_base_type(
    const struct ast* ast,
    ast_id            op,
    union type        base_type,
    union type        target_type,
    const char*       filename,
    const char*       source)
{
    ast_id           base = ast->nodes[op].binop.left;
    struct utf8_view base_tname = type_name(base_type, ast, source);
    struct utf8_view target_tname = type_name(target_type, ast, source);

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, op) == AST_BINOP,
        log_err("type: %d\n", ast_node_type(ast, op)));
    ODBUTIL_DEBUG_ASSERT(
        ast->nodes[op].binop.op == BINOP_POW,
        log_err("op: %d\n", ast->nodes[op].binop.op));

    log_flc(filename, source, ast_loc(ast, base));
    log_err(
        "Incompatible base type {emph0:%.*s} can't be converted to "
        "{emph1:%.*s}.\n",
        base_tname.len,
        base_tname.data + base_tname.off,
        target_tname.len,
        target_tname.data + target_tname.off);
    log_excerpt_2(
        source,
        ast_loc(ast, base),
        ast->nodes[op].binop.op_location,
        base_tname,
        empty_utf8_view(),
        0,
        1);
    log_note(
        "The base can be a {emph1:%s} or {emph1:%s}.\n",
        primitive_type_name(TYPE_F32),
        primitive_type_name(TYPE_F64));

    return -1;
}

int
err_binop_pow_incompatible_exponent_type(
    const struct ast* ast,
    ast_id            op,
    union type        exp_type,
    union type        target_type,
    const char*       filename,
    const char*       source)
{
    ast_id           base = ast->nodes[op].binop.left;
    ast_id           exp = ast->nodes[op].binop.right;
    struct utf8_view exp_tname = type_name(exp_type, ast, source);
    struct utf8_view target_tname = type_name(target_type, ast, source);

    log_flc(filename, source, ast_loc(ast, exp));
    log_err(
        "Incompatible exponent type {emph0:%.*s} can't be converted to "
        "{emph1:%.*s}.\n",
        exp_tname.len,
        exp_tname.data + exp_tname.off,
        target_tname.len,
        target_tname.data + target_tname.off);
    log_excerpt_binop(
        source,
        ast_loc(ast, base),
        ast->nodes[op].binop.op_location,
        ast_loc(ast, exp),
        empty_utf8_view(),
        exp_tname);
    log_help(
        "The exponent can be an {emph1:%s}, {emph1:%s} or {emph1:%s}.\n",
        primitive_type_name(TYPE_I32),
        primitive_type_name(TYPE_F32),
        primitive_type_name(TYPE_F64));

    return -1;
}

int
err_boolean_invalid_evaluation(
    const struct ast* ast,
    ast_id            expr,
    const char*       filename,
    const char*       source)
{
    struct utf8_view expr_tname
        = type_name(ast_type_info(ast, expr), ast, source);

    log_flc(filename, source, ast_loc(ast, expr));
    log_err(
        "Cannot evaluate {emph0:%.*s} as a boolean expression.\n",
        expr_tname.len,
        expr_tname.data + expr_tname.off);
    log_excerpt_1(source, ast_loc(ast, expr), expr_tname, 0);

    return -1;
}

int
err_cast_incompatible_types(
    const struct ast* ast,
    ast_id            cast,
    const char*       filename,
    const char*       source)
{
    ast_id           expr = ast->nodes[cast].cast.expr;
    union type       source_type = ast_type_info(ast, expr);
    union type       target_type = ast_type_info(ast, cast);
    struct utf8_view source_tname = type_name(source_type, ast, source);
    struct utf8_view target_tname = type_name(target_type, ast, source);

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, cast) == AST_CAST,
        log_err("type: %d\n", ast_node_type(ast, cast)));

    log_flc(filename, source, ast_loc(ast, cast));
    log_err(
        "Cannot cast from {emph0:%.*s} to {emph1:%.*s}: Types are "
        "incompatible\n",
        source_tname.len,
        source_tname.data + source_tname.off,
        target_tname.len,
        target_tname.data + target_tname.off);
    log_excerpt_2(
        source,
        ast_loc(ast, expr),
        ast_loc(ast, cast),
        source_tname,
        target_tname,
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
    struct utf8_span param_type_loc;
    struct utf8_view arg_tname
        = type_name(ast_type_info(ast, arg), ast, source);
    struct utf8_view param_tname
        = type_name(ast_type_info(ast, param), ast, source);

    ODBUTIL_DEBUG_ASSERT(arg > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(param > -1, (void)0);
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, param) == AST_PARAM,
        log_err("type: %d\n", ast_node_type(ast, param)));

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, ast->nodes[param].param.identifier)
            == AST_IDENTIFIER,
        log_err(
            "type: %d\n",
            ast_node_type(ast, ast->nodes[param].param.identifier)));

    param_type_loc = ast->nodes[param].param.as > -1
                         ? ast_loc(ast, ast->nodes[param].param.as)
                         : ast_loc(ast, param);

    log_flc(filename, source, ast_loc(ast, arg));
    log_err(
        "Cannot convert %d%s argument from {emph0:%.*s} to {emph1:%.*s} in "
        "function call. Types are incompatible.\n",
        arg_num,
        arg_num == 1   ? "st"
        : arg_num == 2 ? "nd"
        : arg_num == 3 ? "rd"
                       : "th",
        arg_tname.len,
        arg_tname.data + arg_tname.off,
        param_tname.len,
        param_tname.data + param_tname.off);
    log_excerpt_1(source, ast_loc(ast, arg), arg_tname, 1);

    log_flc(filename, source, param_type_loc);
    log_note("Function parameter was declared here:\n");
    log_excerpt_1(source, param_type_loc, empty_utf8_view(), 0);

    return -1;
}

int
err_func_redefinition(
    const struct ast* func_ast,
    ast_id            func,
    const char*       filename,
    const char*       source,
    const struct ast* first_ast,
    ast_id            first_func,
    const char*       first_filename,
    const char*       first_source)
{
    ast_id           identifier, first_identifier;
    struct utf8_span name;
    struct utf8_span loc, first_loc;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(func_ast, func) == AST_FUNC1,
        log_err("type: %d\n", ast_node_type(func_ast, func)));
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(first_ast, first_func) == AST_FUNC1,
        log_err("type: %d\n", ast_node_type(first_ast, first_func)));

    identifier = func_ast->nodes[func].func1.identifier;
    first_identifier = first_ast->nodes[first_func].func1.identifier;
    name = func_ast->nodes[identifier].identifier.name;
    loc = ast_loc(func_ast, identifier);
    first_loc = ast_loc(first_ast, first_identifier);

    log_flc(filename, source, loc);
    log_err(
        "Redefinition of function {quote:%.*s}.\n",
        name.len,
        source + name.off);
    log_excerpt_1(source, loc, empty_utf8_view(), 0);

    log_flc(first_filename, first_source, first_loc);
    log_note("Previously defined here:\n");
    log_excerpt_1(first_source, first_loc, empty_utf8_view(), 0);

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
    struct utf8_view func_tname, ret_tname;

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
    func_tname = type_name(ast_type_info(ast, func), ast, source);
    ret_tname = type_name(ast_type_info(ast, retval), ast, source);

    log_flc(filename, source, ast_loc(ast, retval));
    log_err(
        "Cannot convert {emph0:%.*s} to {emph1:%.*s} in function return. Types "
        "are "
        "incompatible.\n",
        ret_tname.len,
        ret_tname.data + ret_tname.off,
        func_tname.len,
        func_tname.data + func_tname.off);
    log_excerpt_1(source, ast_loc(ast, retval), ret_tname, 0);

    log_flc(filename, source, ret_type_loc);
    log_note("Function return type was declared here:\n");
    log_excerpt_1(source, ret_type_loc, empty_utf8_view(), 1);

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
    log_excerpt_1(source, ret_loc, empty_utf8_view(), 0);

    log_flc(filename, source, ret_type_loc);
    log_note("Function return type was declared here:\n");
    log_excerpt_1(source, ret_type_loc, empty_utf8_view(), 0);

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
    log_excerpt_1(source, inner_name, empty_utf8_view(), 0);

    log_flc(filename, source, outer_name);
    log_note("Previously defined here:\n");
    log_excerpt_1(source, outer_name, empty_utf8_view(), 0);

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
        log_excerpt_1(source, ast_loc(ast, cont), empty_utf8_view(), 0);
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
        log_excerpt_1(source, ast->nodes[cont].cont.name, empty_utf8_view(), 0);

        if (name.len)
        {
            log_flc(filename, source, name);
            log_help(
                "Did you mean {quote:%.*s}?\n", name.len, source + name.off);
            log_excerpt_1(source, name, empty_utf8_view(), 0);
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
    log_excerpt_1(source, ast_loc(ast, exit), empty_utf8_view(), 0);

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
    log_excerpt_1(
        source, ast->nodes[exit].loop_exit.name, empty_utf8_view(), 0);

    if (name.len)
    {
        log_flc(filename, source, name);
        log_help("Did you mean {quote:%.*s}?\n", name.len, source + name.off);
        log_excerpt_1(source, name, empty_utf8_view(), 0);
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
    struct utf8_view ins = empty_utf8_view();
    struct utf8_view ann = empty_utf8_view();
    struct utf8_span loc1
        = utf8_span_union(ast_loc(ast, begin), ast_loc(ast, end));
    struct utf8_span     loc2 = ast_loc(ast, step);
    struct log_highlight hl[]
        = {{ins, ann, loc1, LOG_HIGHLIGHT, LOG_MARKERS, 0},
           {ins, ann, loc2, LOG_HIGHLIGHT, LOG_MARKERS, 0},
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
err_param_redeclaration(
    const struct ast* ast,
    struct utf8_span  name,
    ast_id            first_occurrence,
    const char*       filename,
    const char*       source)
{
    struct utf8_span first_name;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, first_occurrence) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, first_occurrence)));
    first_name = ast->nodes[first_occurrence].identifier.name;

    log_flc(filename, source, name);
    log_err(
        "Parameter {quote:%.*s} already exists.\n",
        name.len,
        source + name.off);
    log_excerpt_1(source, name, empty_utf8_view(), 0);

    log_flc(filename, source, first_name);
    log_note("Previously defined here:\n");
    log_excerpt_1(source, first_name, empty_utf8_view(), 0);

    return -1;
}

int
err_select_incompatible_types(
    const struct ast* ast,
    ast_id            select,
    ast_id            case_,
    const char*       filename,
    const char*       source)
{
    ast_id           select_expr = ast->nodes[select].select.expr;
    ast_id           case_expr = ast->nodes[case_].case_.expr;
    union type       select_type = ast_type_info(ast, select_expr);
    union type       case_type = ast_type_info(ast, case_expr);
    struct utf8_view select_tname = type_name(select_type, ast, source);
    struct utf8_view case_tname = type_name(case_type, ast, source);
    struct utf8_span select_loc = ast_loc(ast, select_expr);
    struct utf8_span case_loc = ast_loc(ast, case_expr);

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, select) == AST_SELECT,
        log_err("type: %d\n", ast_node_type(ast, select)));
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, case_) == AST_CASE,
        log_err("type: %d\n", ast_node_type(ast, case_)));

    log_flc(filename, source, case_loc);
    log_err(
        "Invalid conversion from {emph0:%.*s} to {emph1:%.*s} in select "
        "statement. Types are incompatible.\n",
        case_tname.len,
        case_tname.data + case_tname.off,
        select_tname.len,
        select_tname.data + select_tname.off);
    log_excerpt_1(source, case_loc, case_tname, 0);

    log_flc(filename, source, select_loc);
    log_excerpt_1(source, select_loc, select_tname, 1);

    return -1;
}

int
err_select_duplicate_default(
    const struct ast* ast,
    ast_id            default_case,
    ast_id            first_default_case,
    const char*       filename,
    const char*       source)
{
    struct utf8_span loc, first_loc;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, default_case) == AST_CASE,
        log_err("type: %d\n", ast_node_type(ast, default_case)));
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, first_default_case) == AST_CASE,
        log_err("type: %d\n", ast_node_type(ast, first_default_case)));

    loc = ast->nodes[default_case].case_.case_loc;
    first_loc = ast->nodes[first_default_case].case_.case_loc;

    log_flc(filename, source, loc);
    log_err("Multiple default cases in select statement.\n");
    log_excerpt_1(source, loc, empty_utf8_view(), 0);

    log_flc(filename, source, first_loc);
    log_note("First default case defined here:\n");
    log_excerpt_1(source, first_loc, empty_utf8_view(), 0);

    return -1;
}

int
err_unterminated_remark(
    struct utf8_span location, const char* filename, const char* source)
{
    log_flc(filename, source, location);
    log_err("Unterminated remark.\n");
    log_excerpt_1(source, location, cstr_utf8_view("Remark starts here."), 0);

    return -1;
}

int
err_var_decl_init_incompatible_types(
    const struct ast* ast,
    ast_id            var_decl,
    const char*       filename,
    const char*       source)
{
    ast_id           init_expr;
    struct utf8_view decl_tname, init_tname;

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, var_decl),
        log_err("type: %d\n", ast_node_type(ast, var_decl)));

    init_expr = ast->nodes[var_decl].var_decl1.init_expr;
    ODBUTIL_DEBUG_ASSERT(init_expr > -1, (void)0);

    decl_tname = type_name(ast_type_info(ast, var_decl), ast, source);
    init_tname = type_name(ast_type_info(ast, init_expr), ast, source);

    log_flc(filename, source, ast_loc(ast, init_expr));
    log_err(
        "Cannot initialize {emph0:%.*s} with a {emph1:%.*s}. Types are "
        "incompatible.\n",
        decl_tname.len,
        decl_tname.data + decl_tname.off,
        init_tname.len,
        init_tname.data + init_tname.off);
    log_excerpt_2(
        source,
        ast_loc(ast, var_decl),
        ast_loc(ast, init_expr),
        decl_tname,
        init_tname,
        0,
        1);

    return -1;
}

int
err_var_decl_redeclaration(
    const struct ast* ast,
    struct utf8_span  name,
    const char*       filename,
    const char*       source,
    const struct ast* first_ast,
    ast_id            first_occurrence,
    const char*       first_filename,
    const char*       first_source)
{
    struct utf8_span first_name;
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(first_ast, first_occurrence) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(first_ast, first_occurrence)));
    first_name = first_ast->nodes[first_occurrence].identifier.name;

    log_flc(filename, source, name);
    log_err(
        "Variable {quote:%.*s} already exists.\n", name.len, source + name.off);
    log_excerpt_1(source, name, empty_utf8_view(), 0);

    log_flc(first_filename, first_source, first_name);
    log_note("Previously defined here:\n");
    log_excerpt_1(first_source, first_name, empty_utf8_view(), 0);

    return -1;
}

int
err_udt_decl_redeclaration(
    const struct ast* ast,
    struct utf8_span  name,
    const char*       filename,
    const char*       source,
    const struct ast* first_ast,
    ast_id            first_occurrence,
    const char*       first_filename,
    const char*       first_source)
{
    ast_id           identifier;
    struct utf8_span first_name;

    switch (ast_node_type(first_ast, first_occurrence))
    {
        case AST_UDT_DECL:
            identifier
                = first_ast->nodes[first_occurrence].udt_decl.type_identifier;
            break;
        case AST_FUNC_POLY: {
            ast_id func1 = first_ast->nodes[first_occurrence].func_poly.func;
            identifier = first_ast->nodes[func1].func1.identifier;
            break;
        }
        case AST_FUNC1:
            identifier = first_ast->nodes[first_occurrence].func1.identifier;
            break;

        default:
            ODBUTIL_DEBUG_ASSERT(
                0,
                log_err(
                    "type: %d\n", ast_node_type(first_ast, first_occurrence)));
            identifier = -1;
            break;
    }
    /* Fallback to just using the location as the "name" */
    first_name = identifier ? first_ast->nodes[identifier].identifier.name
                            : ast_loc(first_ast, first_occurrence);

    log_flc(filename, source, name);
    log_err(
        "User-Defined Type {quote:%.*s} already exists.\n",
        name.len,
        source + name.off);
    log_excerpt_1(source, name, empty_utf8_view(), 0);

    log_flc(first_filename, first_source, first_name);
    log_note("Previously defined here:\n");
    log_excerpt_1(first_source, first_name, empty_utf8_view(), 0);

    return -1;
}

int
err_udt_not_found(
    const struct ast* ast,
    struct utf8_span  name,
    const char*       filename,
    const char*       source)
{
    log_flc(filename, source, name);
    log_err(
        "User-Defined Type {quote:%.*s} not found.\n",
        name.len,
        source + name.off);
    log_excerpt_1(source, name, empty_utf8_view(), 0);

    return -1;
}

int
err_udt_member_not_found(
    const struct ast* ast,
    struct utf8_span  name,
    const char*       filename,
    const char*       source)
{
    log_flc(filename, source, name);
    log_err(
        "Member {quote:%.*s} not found in User-Defined Type.\n",
        name.len,
        source + name.off);
    log_excerpt_1(source, name, empty_utf8_view(), 0);

    return -1;
}

int
err_udt_is_not_udt(
    const struct ast* ast,
    struct utf8_span  name,
    const char*       filename,
    const char*       source)
{
    log_flc(filename, source, name);
    log_err(
        "{quote:%.*s} is not a User-Defined Type.\n",
        name.len,
        source + name.off);
    log_excerpt_1(source, name, empty_utf8_view(), 0);

    return -1;
}

void
warn_assignment_implicit_conversion(
    const struct ast* ast,
    ast_id            ass,
    ast_id            first_occurrence,
    const char*       filename,
    const char*       source)
{
    ast_id           lhs = ast->nodes[ass].assignment.lvalue;
    ast_id           rhs = ast->nodes[ass].assignment.expr;
    struct utf8_view lhs_tname
        = type_name(ast_type_info(ast, lhs), ast, source);
    struct utf8_view rhs_tname
        = type_name(ast_type_info(ast, rhs), ast, source);

    ODBUTIL_DEBUG_ASSERT(
        first_occurrence == -1
            || ast_node_type(ast, first_occurrence) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, first_occurrence)));

    log_flc(filename, source, ast_loc(ast, rhs));
    log_warn(
        "Implicit conversion from {emph1:%.*s} to {emph0:%.*s} in "
        "assignment.\n",
        rhs_tname.len,
        rhs_tname.data + rhs_tname.off,
        lhs_tname.len,
        lhs_tname.data + lhs_tname.off);
    log_excerpt_binop(
        source,
        ast_loc(ast, lhs),
        ast->nodes[ass].assignment.op_location,
        ast_loc(ast, rhs),
        lhs_tname,
        rhs_tname);

    if (first_occurrence > -1)
    {
        struct utf8_span first_name
            = ast->nodes[first_occurrence].identifier.name;
        log_flc(filename, source, first_name);
        log_note(
            "{emph0:%.*s} was previously declared as {emph0:%.*s} here:\n",
            first_name.len,
            source + first_name.off,
            lhs_tname.len,
            lhs_tname.data + lhs_tname.off);
        log_excerpt_1(source, first_name, lhs_tname, 0);
    }

    if (type_is_primitive(ast_type_info(ast, lhs)))
        help_insert_explicit_cast(
            source, ast_loc(ast, rhs), ast_type_info(ast, lhs).primitive);
}

void
warn_assignment_truncation(
    const struct ast* ast,
    ast_id            ass,
    ast_id            first_occurrence,
    const char*       filename,
    const char*       source)
{
    ast_id           lhs = ast->nodes[ass].assignment.lvalue;
    ast_id           rhs = ast->nodes[ass].assignment.expr;
    struct utf8_view lhs_tname
        = type_name(ast_type_info(ast, lhs), ast, source);
    struct utf8_view rhs_tname
        = type_name(ast_type_info(ast, rhs), ast, source);

    ODBUTIL_DEBUG_ASSERT(
        first_occurrence == -1
            || ast_node_type(ast, first_occurrence) == AST_IDENTIFIER,
        log_err("type: %d\n", ast_node_type(ast, first_occurrence)));

    log_flc(filename, source, ast_loc(ast, rhs));
    log_warn(
        "Value is truncated in conversion from {emph1:%.*s} to {emph0:%.*s} in "
        "assignment.\n",
        rhs_tname.len,
        rhs_tname.data + rhs_tname.off,
        lhs_tname.len,
        lhs_tname.data + lhs_tname.off);
    log_excerpt_binop(
        source,
        ast_loc(ast, lhs),
        ast->nodes[ass].assignment.op_location,
        ast_loc(ast, rhs),
        lhs_tname,
        rhs_tname);

    if (first_occurrence > -1)
    {
        struct utf8_span first_name
            = ast->nodes[first_occurrence].identifier.name;
        log_flc(filename, source, first_name);
        log_note(
            "{emph0:%.*s} was previously declared as {emph0:%.*s} here:\n",
            first_name.len,
            source + first_name.off,
            lhs_tname.len,
            lhs_tname.data + lhs_tname.off);
        log_excerpt_1(source, first_name, lhs_tname, 0);
    }

    if (type_is_primitive(ast_type_info(ast, lhs)))
        help_insert_explicit_cast(
            source, ast_loc(ast, rhs), ast_type_info(ast, lhs).primitive);
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
    ast_id           lhs = ast->nodes[op].binop.left;
    ast_id           rhs = ast->nodes[op].binop.right;
    union type       source_type = ast_type_info(ast, source_node);
    union type       target_type = ast_type_info(ast, target_node);
    struct utf8_view source_tname = type_name(source_type, ast, source);
    struct utf8_view target_tname = type_name(target_type, ast, source);

    log_flc(filename, source, ast_loc(ast, op));
    log_warn(
        "Implicit conversion from {emph0:%.*s} to {emph1:%.*s} in binary "
        "expression.\n",
        source_tname.len,
        source_tname.data + source_tname.off,
        target_tname.len,
        target_tname.data + target_tname.off);
    log_excerpt_binop(
        source,
        ast_loc(ast, lhs),
        ast->nodes[op].binop.op_location,
        ast_loc(ast, rhs),
        lhs == source_node ? source_tname : target_tname,
        lhs == source_node ? target_tname : source_tname);
    if (type_is_primitive(target_type))
        help_insert_explicit_cast(
            source, ast_loc(ast, source_node), target_type.primitive);
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
    ast_id           lhs = ast->nodes[op].binop.left;
    ast_id           rhs = ast->nodes[op].binop.right;
    union type       source_type = ast_type_info(ast, source_node);
    union type       target_type = ast_type_info(ast, target_node);
    struct utf8_view source_tname = type_name(source_type, ast, source);
    struct utf8_view target_tname = type_name(target_type, ast, source);

    log_flc(filename, source, ast_loc(ast, op));
    log_warn(
        "Value is truncated when converting from {emph0:%.*s} to {emph1:%.*s} "
        "in binary expression.\n",
        source_tname.len,
        source_tname.data + source_tname.off,
        target_tname.len,
        target_tname.data + target_tname.off);
    log_excerpt_binop(
        source,
        ast_loc(ast, lhs),
        ast->nodes[op].binop.op_location,
        ast_loc(ast, rhs),
        lhs == source_node ? source_tname : target_tname,
        lhs == source_node ? target_tname : source_tname);
}

void
warn_binop_pow_base_implicit_conversion(
    const struct ast* ast,
    ast_id            op,
    union type        base_type,
    union type        target_type,
    const char*       filename,
    const char*       source)
{
    ast_id           base = ast->nodes[op].binop.left;
    ast_id           exp = ast->nodes[op].binop.right;
    struct utf8_view base_tname = type_name(base_type, ast, source);
    struct utf8_view target_tname = type_name(target_type, ast, source);
    struct utf8_view exp_tname
        = type_name(ast_type_info(ast, exp), ast, source);

    log_flc(filename, source, ast_loc(ast, base));
    log_warn(
        "Implicit conversion of base value from {emph0:%.*s} to {emph1:%.*s} "
        "in exponentiation.\n",
        base_tname.len,
        base_tname.data + base_tname.off,
        target_tname.len,
        target_tname.data + target_tname.off);
    log_excerpt_binop(
        source,
        ast_loc(ast, base),
        ast->nodes[op].binop.op_location,
        ast_loc(ast, exp),
        base_tname,
        exp_tname);
    log_note(
        "The base can be a {emph1:%s} or {emph1:%s}\n",
        primitive_type_name(TYPE_F32),
        primitive_type_name(TYPE_F64));

    if (type_is_primitive(target_type))
        help_insert_explicit_cast(
            source, ast_loc(ast, base), target_type.primitive);
}

void
warn_binop_pow_base_truncation(
    const struct ast* ast,
    ast_id            op,
    union type        base_type,
    union type        target_type,
    const char*       filename,
    const char*       source)
{
    ast_id           base = ast->nodes[op].binop.left;
    ast_id           exp = ast->nodes[op].binop.right;
    struct utf8_view base_tname = type_name(base_type, ast, source);
    struct utf8_view target_tname = type_name(target_type, ast, source);
    struct utf8_view exp_tname
        = type_name(ast_type_info(ast, exp), ast, source);

    log_flc(filename, source, ast_loc(ast, base));
    log_warn(
        "Base value is truncated when converting from {emph0:%.*s} to "
        "{emph1:%.*s} in exponentiation.\n",
        base_tname.len,
        base_tname.data + base_tname.off,
        target_tname.len,
        target_tname.data + target_tname.off);
    log_excerpt_binop(
        source,
        ast_loc(ast, base),
        ast->nodes[op].binop.op_location,
        ast_loc(ast, exp),
        base_tname,
        exp_tname);
    log_note(
        "The base can be a {emph1:%s} or {emph1:%s}.\n",
        primitive_type_name(TYPE_F32),
        primitive_type_name(TYPE_F64));

    if (type_is_primitive(target_type))
        help_insert_explicit_cast(
            source, ast_loc(ast, base), target_type.primitive);
}

void
warn_binop_pow_exponent_implicit_conversion(
    const struct ast* ast,
    ast_id            op,
    union type        exp_type,
    union type        target_type,
    const char*       filename,
    const char*       source)
{
    ast_id           base = ast->nodes[op].binop.left;
    ast_id           exp = ast->nodes[op].binop.right;
    struct utf8_view target_tname = type_name(target_type, ast, source);
    struct utf8_view exp_tname = type_name(exp_type, ast, source);

    log_flc(filename, source, ast_loc(ast, exp));
    log_warn(
        "Implicit conversion of exponent from {emph1:%.*s} to {emph0:%.*s}.\n",
        exp_tname.len,
        exp_tname.data + exp_tname.off,
        target_tname.len,
        target_tname.data + target_tname.off);
    log_excerpt_binop(
        source,
        ast_loc(ast, base),
        ast->nodes[op].binop.op_location,
        ast_loc(ast, exp),
        empty_utf8_view(),
        exp_tname);

    if (target_type.primitive == TYPE_F32)
        log_help(
            "The exponent needs to be the same type as the base when working "
            "with floating point types.\n");

    if (exp_type.primitive == TYPE_I64 || exp_type.primitive == TYPE_U32)
        log_help(
            "{emph1:INTEGER} is the largest possible integral type for "
            "exponents.\n");

    log_help(
        "The exponent can be an {emph1:%s}, {emph1:%s} or {emph1:%s}.\n",
        primitive_type_name(TYPE_I32),
        primitive_type_name(TYPE_F32),
        primitive_type_name(TYPE_F64));
}
void
warn_binop_pow_exponent_truncation(
    const struct ast* ast,
    ast_id            op,
    union type        exp_type,
    union type        target_type,
    const char*       filename,
    const char*       source)
{
    ast_id           base = ast->nodes[op].binop.left;
    ast_id           exp = ast->nodes[op].binop.right;
    struct utf8_view target_tname = type_name(target_type, ast, source);
    struct utf8_view exp_tname = type_name(exp_type, ast, source);

    log_flc(filename, source, ast_loc(ast, exp));
    log_warn(
        "Exponent value is truncated when converting from {emph1:%.*s} to "
        "{emph0:%.*s}.\n",
        exp_tname.len,
        exp_tname.data + exp_tname.off,
        target_tname.len,
        target_tname.data + target_tname.off);
    log_excerpt_binop(
        source,
        ast_loc(ast, base),
        ast->nodes[op].binop.op_location,
        ast_loc(ast, exp),
        target_type.primitive == TYPE_F32
            ? cstr_utf8_view(primitive_type_name(TYPE_F32))
            : empty_utf8_view(),
        exp_tname);

    if (target_type.primitive == TYPE_F32)
        log_help(
            "The exponent is always converted to the same type as the base "
            "when using floating point exponents.\n");

    if (target_type.primitive == TYPE_I32)
        log_help(
            "{emph1:INTEGER} is the largest possible integral type for "
            "exponents.\n");

    log_help(
        "The exponent can be an {emph1:%s}, {emph1:%s} or {emph1:%s}.\n",
        primitive_type_name(TYPE_I32),
        primitive_type_name(TYPE_F32),
        primitive_type_name(TYPE_F64));
}

void
warn_boolean_implicit_evaluation(
    const struct ast* ast,
    ast_id            expr,
    const char*       filename,
    const char*       source)
{
    struct utf8_view ins1 = cstr_utf8_view(" <> 0");
    struct utf8_view ins2 = cstr_utf8_view(" <> 0.0f");
    struct utf8_view ins3 = cstr_utf8_view(" <> 0.0");
    struct utf8_view ins4 = cstr_utf8_view(" <> \"\"");
    struct utf8_view ann = empty_utf8_view();
    struct utf8_span expr_loc = ast_loc(ast, expr);
    struct utf8_view expr_tname
        = type_name(ast_type_info(ast, expr), ast, source);

    utf8_idx expr_start = expr_loc.off;
    utf8_idx expr_end = expr_start + expr_loc.len;

    struct log_highlight hl_int[]
        = {{ins1, ann, {expr_end, ins1.len}, LOG_INSERT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};
    struct log_highlight hl_float[]
        = {{ins2, ann, {expr_end, ins2.len}, LOG_INSERT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};
    struct log_highlight hl_double[]
        = {{ins3, ann, {expr_end, ins3.len}, LOG_INSERT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};
    struct log_highlight hl_string[]
        = {{ins4, ann, {expr_end, ins4.len}, LOG_INSERT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};

    log_flc(filename, source, ast_loc(ast, expr));
    log_warn(
        "Implicit evaluation of {emph0:%.*s} as a boolean expression.\n",
        expr_tname.len,
        expr_tname.data + expr_tname.off);
    log_excerpt_1(source, ast_loc(ast, expr), expr_tname, 0);

    switch (ast_type_info(ast, expr).primitive)
    {
        case TYPE_I64: /* fallthrough */
        case TYPE_U32: /* fallthrough */
        case TYPE_I32: /* fallthrough */
        case TYPE_U16: /* fallthrough */
        case TYPE_U8:
            log_help("You can make it explicit by changing it to:\n");
            log_excerpt(source, hl_int);
            break;
        case TYPE_F32:
            log_help("You can make it explicit by changing it to:\n");
            log_excerpt(source, hl_float);
            break;
        case TYPE_F64:
            log_help("You can make it explicit by changing it to:\n");
            log_excerpt(source, hl_double);
            break;
        case TYPE_STRING:
            log_help("You can make it explicit by changing it to:\n");
            log_excerpt(source, hl_string);
            break;
        case TYPE_INVALID:
        case TYPE_VOID:
        case TYPE_BOOL: break;
    }
}

void
warn_cmd_return_value_ignored(
    const struct ast* ast, ast_id cmd, const char* filename, const char* source)
{
    struct utf8_view ins = cstr_utf8_view(" AS VOID");
    struct utf8_view ann = empty_utf8_view();
    struct utf8_span loc = ast_loc(ast, cmd);
    union type       type = ast_type_info(ast, cmd);

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, cmd) == AST_COMMAND,
        log_err("type: %d\n", ast_node_type(ast, cmd)));

    struct log_highlight hl[]
        = {{ins, ann, {loc.off + loc.len, ins.len}, LOG_INSERT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};

    log_flc(filename, source, loc);
    log_warn("Return value of command is ignored.\n");
    log_excerpt_1(source, loc, type_name(type, ast, source), 0);

    log_help(
        "If this was intended, you can cast the return value to VOID to "
        "silence this warning:\n");
    log_excerpt(source, hl);
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
    struct utf8_view arg_tname
        = type_name(ast_type_info(ast, arg), ast, source);
    struct utf8_view param_tname
        = type_name(ast_type_info(ast, param), ast, source);

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
        "Implicit conversion of %d%s argument from {emph0:%.*s} to "
        "{emph1:%.*s} in function call.\n",
        arg_num,
        arg_num == 1   ? "st"
        : arg_num == 2 ? "nd"
        : arg_num == 3 ? "rd"
                       : "th",
        arg_tname.len,
        arg_tname.data + arg_tname.off,
        param_tname.len,
        param_tname.data + param_tname.off);
    log_excerpt_1(source, ast_loc(ast, arg), arg_tname, 1);

    log_flc(filename, source, param_type_loc);
    log_note("Function parameter type is declared here:\n");
    log_excerpt_1(source, param_type_loc, empty_utf8_view(), 0);

    if (type_is_primitive(ast_type_info(ast, param)))
        help_insert_explicit_cast(
            source, ast_loc(ast, arg), ast_type_info(ast, param).primitive);
}

void
warn_func_call_return_value_ignored(
    const struct ast* ast,
    ast_id            func,
    const char*       filename,
    const char*       source)
{
    struct utf8_view ins = cstr_utf8_view(" AS VOID");
    struct utf8_view ann = empty_utf8_view();
    struct utf8_span loc = ast_loc(ast, func);
    union type       type = ast_type_info(ast, func);

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, func) == AST_FUNC_CALL,
        log_err("type: %d\n", ast_node_type(ast, func)));

    struct log_highlight hl[]
        = {{ins, ann, {loc.off + loc.len, ins.len}, LOG_INSERT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};

    log_flc(filename, source, loc);
    log_warn("Return value of function call is ignored.\n");
    log_excerpt_1(source, loc, type_name(type, ast, source), 0);

    log_help(
        "If this was intended, you can cast the return value to VOID to "
        "silence this warning:\n");
    log_excerpt(source, hl);
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
    struct utf8_view arg_tname
        = type_name(ast_type_info(ast, arg), ast, source);
    struct utf8_view param_tname
        = type_name(ast_type_info(ast, param), ast, source);

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
        "Value is truncated when converting %d%s argument from {emph0:%.*s} to "
        "{emph1:%.*s} in function call.\n",
        arg_num,
        arg_num == 1   ? "st"
        : arg_num == 2 ? "nd"
        : arg_num == 3 ? "rd"
                       : "th",
        arg_tname.len,
        arg_tname.data + arg_tname.off,
        param_tname.len,
        param_tname.data + param_tname.off);
    log_excerpt_1(source, ast_loc(ast, arg), arg_tname, 1);

    log_flc(filename, source, param_type_loc);
    log_note("Function return type was declared here:\n");
    log_excerpt_1(source, param_type_loc, empty_utf8_view(), 0);

    if (type_is_primitive(ast_type_info(ast, param)))
        help_insert_explicit_cast(
            source, ast_loc(ast, arg), ast_type_info(ast, param).primitive);
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
    struct utf8_view func_tname, ret_tname;

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
    func_tname = type_name(ast_type_info(ast, func), ast, source);
    ret_tname = type_name(ast_type_info(ast, retval), ast, source);

    log_flc(filename, source, ast_loc(ast, retval));
    log_warn(
        "Implicit conversion from {emph0:%.*s} to {emph1:%.*s} in function "
        "return.\n",
        ret_tname.len,
        ret_tname.data + ret_tname.off,
        func_tname.len,
        func_tname.data + func_tname.off);
    log_excerpt_1(source, ast_loc(ast, retval), ret_tname, 0);

    log_flc(filename, source, ret_type_loc);
    log_note("Function return type was declared here:\n");
    log_excerpt_1(source, ret_type_loc, empty_utf8_view(), 1);

    if (type_is_primitive(ast_type_info(ast, identifier)))
        help_insert_explicit_cast(
            source,
            ast_loc(ast, retval),
            ast_type_info(ast, identifier).primitive);
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
    struct utf8_view func_tname, ret_tname;

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
    func_tname = type_name(ast_type_info(ast, func), ast, source);
    ret_tname = type_name(ast_type_info(ast, retval), ast, source);

    log_flc(filename, source, ast_loc(ast, retval));
    log_warn(
        "Value is truncated when converting from {emph1:%.*s} to {emph0:%.*s} "
        "in function return.\n",
        ret_tname.len,
        ret_tname.data + ret_tname.off,
        func_tname.len,
        func_tname.data + func_tname.off);
    log_excerpt_1(source, ast_loc(ast, retval), ret_tname, 0);

    log_flc(filename, source, ret_type_loc);
    log_note("Function return type was declared here:\n");
    log_excerpt_1(source, ret_type_loc, empty_utf8_view(), 1);

    if (type_is_primitive(ast_type_info(ast, identifier)))
        help_insert_explicit_cast(
            source,
            ast_loc(ast, retval),
            ast_type_info(ast, identifier).primitive);
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
    log_excerpt_1(source, exit_loc, empty_utf8_view(), 0);

    log_flc(filename, source, name);
    log_note("This exit statement will exit the inner loop:\n");
    log_excerpt_1(source, name, empty_utf8_view(), 0);

    log_flc(filename, source, outer_name);
    log_note("Outer loop with the same name defined here:\n");
    log_excerpt_1(source, outer_name, empty_utf8_view(), 0);
}

void
warn_loop_for_default_step_may_be_incorrect(
    const struct ast* ast,
    ast_id            begin,
    ast_id            end,
    const char*       filename,
    const char*       source)
{
    struct utf8_view ins1 = cstr_utf8_view(" STEP 1");
    struct utf8_view ins2 = cstr_utf8_view(" STEP -1");
    struct utf8_view ann = empty_utf8_view();

    struct utf8_span loc
        = utf8_span_union(ast_loc(ast, begin), ast_loc(ast, end));
    utf8_idx loc_end = loc.off + loc.len;

    struct log_highlight hl_step_forwards[]
        = {{ins1, ann, {loc_end, ins1.len}, LOG_INSERT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};
    struct log_highlight hl_step_backwards[]
        = {{ins2, ann, {loc_end, ins2.len}, LOG_INSERT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};

    log_flc(filename, source, loc);
    log_warn("For-loop direction may be incorrect.\n");
    log_excerpt_1(source, loc, empty_utf8_view(), 0);

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
    struct utf8_view ins = empty_utf8_view();
    struct utf8_view ann = empty_utf8_view();

    struct utf8_span loc1
        = utf8_span_union(ast_loc(ast, begin), ast_loc(ast, end));
    struct utf8_span loc2 = ast_loc(ast, step);

    struct log_highlight hl[]
        = {{ins, ann, loc1, LOG_HIGHLIGHT, LOG_MARKERS, 0},
           {ins, ann, loc2, LOG_HIGHLIGHT, LOG_MARKERS, 0},
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
    struct utf8_view ins = cstr_utf8_view(" STEP 1");
    struct utf8_view ann = empty_utf8_view();

    struct utf8_span loc
        = utf8_span_union(ast_loc(ast, begin), ast_loc(ast, end));
    utf8_idx loc_end = loc.off + loc.len;

    struct log_highlight hl[]
        = {{ins, ann, {loc_end, ins.len}, LOG_INSERT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};

    log_flc(filename, source, loc);
    log_warn(
        "For-loop does nothing, because it STEPs in the wrong direction.\n");
    log_excerpt_1(source, loc, empty_utf8_view(), 0);
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
    log_excerpt_1(source, ast_loc(ast, next), empty_utf8_view(), 0);

    log_flc(filename, source, ast_loc(ast, loop_var));
    log_note("Loop variable declared here:\n");
    log_excerpt_1(source, ast_loc(ast, loop_var), empty_utf8_view(), 0);
}

void
warn_select_implicit_conversion(
    const struct ast* ast,
    ast_id            select,
    ast_id            case_,
    const char*       filename,
    const char*       source)
{
    ast_id           select_expr = ast->nodes[select].select.expr;
    ast_id           case_expr = ast->nodes[case_].case_.expr;
    union type       select_type = ast_type_info(ast, select_expr);
    union type       case_type = ast_type_info(ast, case_expr);
    struct utf8_view select_tname = type_name(select_type, ast, source);
    struct utf8_view case_tname = type_name(case_type, ast, source);
    struct utf8_span select_loc = ast_loc(ast, select_expr);
    struct utf8_span case_loc = ast_loc(ast, case_expr);

    utf8_idx             loc_end = case_loc.off + case_loc.len;
    struct utf8_view     ins1 = cstr_utf8_view(" AS ");
    struct utf8_view     ins2 = select_tname;
    struct utf8_view     ann = empty_utf8_view();
    struct log_highlight hl[]
        = {{ins1, ann, {loc_end, ins1.len}, LOG_INSERT, "^~~", 0},
           {ins2, ann, {loc_end, ins2.len}, LOG_INSERT, "~~<", 0},
           LOG_HIGHLIGHT_SENTINAL};

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, select) == AST_SELECT,
        log_err("type: %d\n", ast_node_type(ast, select)));
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, case_) == AST_CASE,
        log_err("type: %d\n", ast_node_type(ast, case_)));

    log_flc(filename, source, case_loc);
    log_warn(
        "Implicit conversion from {emph0:%.*s} to {emph1:%.*s} in select "
        "statement.\n",
        case_tname.len,
        case_tname.data + case_tname.off,
        select_tname.len,
        select_tname.data + select_tname.off);
    log_excerpt_1(source, case_loc, case_tname, 0);

    log_flc(filename, source, select_loc);
    log_excerpt_1(source, select_loc, select_tname, 1);

    log_help("Insert an explicit cast to silence this warning:\n");
    log_excerpt(source, hl);
}

void
warn_select_truncation(
    const struct ast* ast,
    ast_id            select,
    ast_id            case_,
    const char*       filename,
    const char*       source)
{
    ast_id           select_expr = ast->nodes[select].select.expr;
    ast_id           case_expr = ast->nodes[case_].case_.expr;
    union type       select_type = ast_type_info(ast, select_expr);
    union type       case_type = ast_type_info(ast, case_expr);
    struct utf8_view select_tname = type_name(select_type, ast, source);
    struct utf8_view case_tname = type_name(case_type, ast, source);
    struct utf8_span select_loc = ast_loc(ast, select_expr);
    struct utf8_span case_loc = ast_loc(ast, case_expr);

    utf8_idx             loc_end = case_loc.off + case_loc.len;
    struct utf8_view     ins1 = cstr_utf8_view(" AS ");
    struct utf8_view     ins2 = select_tname;
    struct utf8_view     ann = empty_utf8_view();
    struct log_highlight hl[]
        = {{ins1, ann, {loc_end, ins1.len}, LOG_INSERT, "^~~", 0},
           {ins2, ann, {loc_end, ins2.len}, LOG_INSERT, "~~<", 0},
           LOG_HIGHLIGHT_SENTINAL};

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, select) == AST_SELECT,
        log_err("type: %d\n", ast_node_type(ast, select)));
    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, case_) == AST_CASE,
        log_err("type: %d\n", ast_node_type(ast, case_)));

    log_flc(filename, source, case_loc);
    log_warn(
        "Case value is truncated when converting from {emph0:%.*s} to "
        "{emph1:%.*s} in select statement.\n",
        case_tname.len,
        case_tname.data + case_tname.off,
        select_tname.len,
        select_tname.data + select_tname.off);
    log_excerpt_1(source, case_loc, case_tname, 0);
    log_flc(filename, source, select_loc);
    log_excerpt_1(source, select_loc, select_tname, 1);

    log_help("Insert an explicit cast to silence this warning:\n");
    log_excerpt(source, hl);
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
    union type       lhs_type = ast_type_info(ast, lhs);
    union type       rhs_type = ast_type_info(ast, rhs);
    struct utf8_view lhs_tname = type_name(lhs_type, ast, source);
    struct utf8_view rhs_tname = type_name(rhs_type, ast, source);

    char             annotation_cstr[2] = {type_to_annotation(rhs_type), '\0'};
    struct utf8_view ins1 = cstr_utf8_view(annotation_cstr);
    struct utf8_view ins2 = cstr_utf8_view(" AS ");
    struct utf8_view ann = empty_utf8_view();
    struct utf8_span lhs_loc = ast_loc(ast, lhs);
    utf8_idx         lhs_end = lhs_loc.off + lhs_loc.len;

    struct log_highlight hl_annotation[]
        = {{ins1, ann, {lhs_end, ins1.len}, LOG_INSERT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};
    struct log_highlight hl_as_type[]
        = {{ins2, ann, {lhs_end, ins2.len}, LOG_INSERT, "^~~", 0},
           {rhs_tname, ann, {lhs_end, rhs_tname.len}, LOG_INSERT, "~~<", 0},
           LOG_HIGHLIGHT_SENTINAL};

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, var_decl) == AST_VAR_DECL1,
        log_err("type: %d\n", ast_node_type(ast, var_decl)));

    log_flc(filename, source, ast_loc(ast, rhs));
    log_warn(
        "Implicit conversion from {emph1:%.*s} to {emph0:%.*s} in variable "
        "initialization.\n",
        rhs_tname.len,
        rhs_tname.data + rhs_tname.off,
        lhs_tname.len,
        lhs_tname.data + lhs_tname.off);
    log_excerpt_binop(
        source,
        ast_loc(ast, lhs),
        op_loc,
        ast_loc(ast, rhs),
        lhs_tname,
        rhs_tname);

    if (annotation_cstr[0] != TA_NONE)
    {
        log_help("Annotate the variable:\n");
        log_excerpt(source, hl_annotation);
    }
    log_help(
        "%sxplicitly declare the type of the variable:\n",
        annotation_cstr[0] != TA_NONE ? "Or e" : "E");
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
    union type       lhs_type = ast_type_info(ast, lhs);
    union type       rhs_type = ast_type_info(ast, rhs);
    struct utf8_view lhs_tname = type_name(lhs_type, ast, source);
    struct utf8_view rhs_tname = type_name(rhs_type, ast, source);

    char             annotation_cstr[2] = {type_to_annotation(rhs_type), '\0'};
    struct utf8_view ins1 = cstr_utf8_view(annotation_cstr);
    struct utf8_view ins2 = cstr_utf8_view(" AS ");
    struct utf8_view ann = empty_utf8_view();
    struct utf8_span lhs_loc = ast_loc(ast, lhs);
    utf8_idx         lhs_end = lhs_loc.off + lhs_loc.len;

    struct log_highlight hl_annotation[]
        = {{ins1, ann, {lhs_end, ins1.len}, LOG_INSERT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};
    struct log_highlight hl_as_type[]
        = {{ins2, ann, {lhs_end, ins2.len}, LOG_INSERT, "^~~", 0},
           {rhs_tname, ann, {lhs_end, rhs_tname.len}, LOG_INSERT, "~~<", 0},
           LOG_HIGHLIGHT_SENTINAL};

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, var_decl) == AST_VAR_DECL1,
        log_err("type: %d\n", ast_node_type(ast, var_decl)));

    log_flc(filename, source, ast_loc(ast, rhs));
    log_warn(
        "Value is truncated in conversion from {emph1:%.*s} to {emph0:%.*s} in "
        "variable initialization.\n",
        rhs_tname.len,
        rhs_tname.data + rhs_tname.off,
        lhs_tname.len,
        lhs_tname.data + lhs_tname.off);
    log_excerpt_binop(
        source,
        ast_loc(ast, lhs),
        op_loc,
        ast_loc(ast, rhs),
        lhs_tname,
        rhs_tname);

    if (annotation_cstr[0] != TA_NONE)
    {
        log_help("Annotate the variable:\n");
        log_excerpt(source, hl_annotation);
    }
    log_help(
        "%sxplicitly declare the type of the variable:\n",
        annotation_cstr[0] != TA_NONE ? "Or e" : "E");
    log_excerpt(source, hl_as_type);
}
