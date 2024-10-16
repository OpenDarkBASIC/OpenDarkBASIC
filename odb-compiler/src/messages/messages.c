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
        "Value is truncated in conversion from {emph2:%s} to "
        "{emph1:%s} in assignment.\n",
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
        "Value is truncated in conversion from {emph2:%s} to "
        "{emph1:%s} in variable initialization.\n",
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
