#pragma once

#include "odb-compiler/semantic/type.h"
#include "odb-util/utf8.h"

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
    ast_id            first_occurrence,
    const char*       filename,
    const char*       source);
int
err_binop_incompatible_types(
    const struct ast* ast,
    ast_id            source_node,
    ast_id            op,
    const char*       filename,
    const char*       source);
int
err_binop_pow_incompatible_base_type(
    const struct ast* ast,
    ast_id            op,
    union type        base_type,
    union type        target_type,
    const char*       filename,
    const char*       source);
int
err_binop_pow_incompatible_exponent_type(
    const struct ast* ast,
    ast_id            op,
    union type        exp_type,
    union type        target_type,
    const char*       filename,
    const char*       source);
int
err_boolean_invalid_evaluation(
    const struct ast* ast,
    ast_id            expr,
    const char*       filename,
    const char*       source);
int
err_cast_incompatible_types(
    const struct ast* ast,
    ast_id            cast,
    const char*       filename,
    const char*       source);
int
err_func_call_incompatible_types(
    const struct ast* ast,
    ast_id            arg,
    ast_id            param,
    int               arg_num,
    const char*       filename,
    const char*       source);
int
err_func_redefinition(
    const struct ast* func_ast,
    ast_id            func,
    const char*       filename,
    const char*       source,
    const struct ast* first_ast,
    ast_id            first_func,
    const char*       first_filename,
    const char*       first_source);
int
err_func_return_incompatible_types(
    const struct ast* ast,
    ast_id            exit,
    ast_id            func,
    const char*       filename,
    const char*       source);
int
err_func_missing_return_value(
    const struct ast* ast,
    ast_id            func,
    struct utf8_span  ret_loc,
    const char*       filename,
    const char*       source);
int
err_loop_duplicate_name(
    const struct ast* ast,
    struct utf8_span  inner_name,
    struct utf8_span  outer_name,
    const char*       filename,
    const char*       source);
int
err_loop_cont(
    const struct ast* ast,
    ast_id            cont,
    ast_id            first_loop,
    const char*       filename,
    const char*       source);
int
err_loop_exit_not_inside_loop(
    const struct ast* ast,
    ast_id            exit,
    const char*       filename,
    const char*       source);
int
err_loop_exit_unknown_name(
    const struct ast* ast,
    ast_id            exit,
    ast_id            first_loop,
    const char*       filename,
    const char*       source);
int
err_loop_for_unknown_direction(
    const struct ast* ast,
    ast_id            begin,
    ast_id            end,
    ast_id            step,
    const char*       filename,
    const char*       source);
int
err_param_redeclaration(
    const struct ast* ast,
    struct utf8_span  name,
    ast_id            first_occurrence,
    const char*       filename,
    const char*       source);
int
err_unterminated_remark(
    struct utf8_span location, const char* filename, const char* source);
int
err_var_decl_init_incompatible_types(
    const struct ast* ast,
    ast_id            var_decl,
    const char*       filename,
    const char*       source);
int
err_var_decl_redeclaration(
    const struct ast* ast,
    struct utf8_span  name,
    const char*       filename,
    const char*       source,
    const struct ast* first_ast,
    ast_id            first_occurrence,
    const char*       first_filename,
    const char*       first_source);
int
err_udt_decl_redeclaration(
    const struct ast* ast,
    struct utf8_span  name,
    const char*       filename,
    const char*       source,
    const struct ast* first_ast,
    ast_id            first_occurrence, /* identifier, func_poly or func1 */
    const char*       first_filename,
    const char*       first_source);
int
err_udt_not_found(
    const struct ast* ast,
    struct utf8_span  name,
    const char*       filename,
    const char*       source);
int
err_udt_member_not_found(
    const struct ast* ast,
    struct utf8_span  name,
    const char*       filename,
    const char*       source);
int
err_udt_is_not_udt(
    const struct ast* ast,
    struct utf8_span  name,
    const char*       filename,
    const char*       source);

void
warn_assignment_implicit_conversion(
    const struct ast* ast,
    ast_id            ass,
    ast_id            first_occurrence,
    const char*       filename,
    const char*       source);
void
warn_assignment_truncation(
    const struct ast* ast,
    ast_id            ass,
    ast_id            first_occurrence,
    const char*       filename,
    const char*       source);
void
warn_binop_implicit_conversion(
    const struct ast* ast,
    ast_id            op,
    ast_id            source_node,
    ast_id            target_node,
    const char*       filename,
    const char*       source);
void
warn_binop_truncation(
    const struct ast* ast,
    ast_id            op,
    ast_id            source_node,
    ast_id            target_node,
    const char*       filename,
    const char*       source);
void
warn_binop_pow_base_implicit_conversion(
    const struct ast* ast,
    ast_id            op,
    union type        base_type,
    union type        target_type,
    const char*       filename,
    const char*       source);
void
warn_binop_pow_base_truncation(
    const struct ast* ast,
    ast_id            op,
    union type        base_type,
    union type        target_type,
    const char*       filename,
    const char*       source);
void
warn_binop_pow_exponent_implicit_conversion(
    const struct ast* ast,
    ast_id            op,
    union type        exp_type,
    union type        target_type,
    const char*       filename,
    const char*       source);
void
warn_binop_pow_exponent_truncation(
    const struct ast* ast,
    ast_id            op,
    union type        exp_type,
    union type        target_type,
    const char*       filename,
    const char*       source);
void
warn_boolean_implicit_evaluation(
    const struct ast* ast,
    ast_id            expr,
    const char*       filename,
    const char*       source);
void
warn_cmd_return_value_ignored(
    const struct ast* ast,
    ast_id            cmd,
    const char*       filename,
    const char*       source);
void
warn_func_call_implicit_conversion(
    const struct ast* ast,
    ast_id            arg,
    ast_id            param,
    int               arg_num,
    const char*       filename,
    const char*       source);
void
warn_func_call_return_value_ignored(
    const struct ast* ast,
    ast_id            cmd,
    const char*       filename,
    const char*       source);
void
warn_func_call_truncation(
    const struct ast* ast,
    ast_id            arg,
    ast_id            param,
    int               arg_num,
    const char*       filename,
    const char*       source);
void
warn_func_return_implicit_conversion(
    const struct ast* ast,
    ast_id            exit,
    ast_id            func,
    const char*       filename,
    const char*       source);
void
warn_func_return_truncation(
    const struct ast* ast,
    ast_id            exit,
    ast_id            func,
    const char*       filename,
    const char*       source);
void
warn_loop_exit_ambiguous_name(
    const struct ast* ast,
    ast_id            exit,
    struct utf8_span  name,
    struct utf8_span  outer_name,
    const char*       filename,
    const char*       source);
void
warn_loop_for_default_step_may_be_incorrect(
    const struct ast* ast,
    ast_id            begin,
    ast_id            end,
    const char*       filename,
    const char*       source);
void
warn_loop_for_wrong_direction(
    const struct ast* ast,
    ast_id            begin,
    ast_id            end,
    ast_id            step,
    const char*       filename,
    const char*       source);
void
warn_loop_for_wrong_direction_no_step(
    const struct ast* ast,
    ast_id            begin,
    ast_id            end,
    const char*       filename,
    const char*       source);
void
warn_loop_for_incorrect_next(
    struct ast* ast,
    ast_id      next,
    ast_id      loop_var,
    const char* filename,
    const char* source);
void
warn_var_decl_implicit_conversion(
    const struct ast* ast,
    ast_id            var_decl,
    const char*       filename,
    const char*       source);
void
warn_var_decl_truncation(
    const struct ast* ast,
    ast_id            var_decl,
    const char*       filename,
    const char*       source);
