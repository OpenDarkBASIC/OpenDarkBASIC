#pragma once

struct ast;
typedef int ast_id;

struct msg_cfg
{
    unsigned warnings_are_errors : 1;
    unsigned warn_sign_change : 1;
    unsigned warn_trueness : 1;
    unsigned warn_int_to_float : 1;
    unsigned warn_bool_promotion : 1;
};

int
err_assignment_incompatible_types(
    const struct ast* ast,
    ast_id            ass,
    ast_id            orig_decl,
    const char*       filename,
    const char*       source);
int
err_initialization_incompatible_types(
    const struct ast* ast,
    ast_id            ass,
    const char*       filename,
    const char*       source);

void
warn_assignment_implicit_conversion(
    const struct ast* ast,
    ast_id            ass,
    ast_id            orig_decl,
    const char*       filename,
    const char*       source);
void
warn_assignment_truncation(
    const struct ast* ast,
    ast_id            ass,
    ast_id            orig_decl,
    const char*       filename,
    const char*       source);
void
warn_initialization_implicit_conversion(
    const struct ast* ast,
    ast_id            ass,
    const char*       filename,
    const char*       source);
void
warn_initialization_truncation(
    const struct ast* ast,
    ast_id            ass,
    const char*       filename,
    const char*       source);
