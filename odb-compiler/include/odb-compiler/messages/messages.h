#pragma once

#include "odb-compiler/semantic/type.h"
#include "odb-util/ospath.h"
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
msg_init(void);

void
msg_deinit(void);

void
log_flc(struct ospathc filename, const char* source, struct utf8_span location);

enum log_highlight_type
{
    LOG_HIGHLIGHT,
    LOG_INSERT,
    LOG_REMOVE,
};

struct log_highlight
{
    /*! Only used in INSERT mode -- The text to insert at offset loc.off
     * The length of the inserted text must equal loc.len. If unused, set to "".
     * DON'T set to NULL. */
    struct utf8_view new_text;
    /*! Annotate the highlighted section with additional information.
     * Can be an empty string, but should not be NULL */
    struct utf8_view annotation;
    /*! If INSERT, then this is the offset in the original text where to insert
     * new_text. Len should be the length of the inserted text.
     * If HIGHLIGHT, then this is the location of the text to highlight. */
    struct utf8_span        loc;
    enum log_highlight_type type;
    /*! The characters to use for underlining the highlighted text. By
     * convention, marker[0]='^', marker[1]='~', marker[2]='<' */
    char marker[3];
    /*! Controls the color of the highlighted text. If multiple locations share
     * the same highlight group, they will be colored the same. */
    char group;
};
#define LOG_HIGHLIGHT_SENTINAL                                                 \
    {{NULL, 0, 0},                                                             \
     {NULL, 0, 0},                                                             \
     {0, 0},                                                                   \
     (enum log_highlight_type)0,                                               \
     {'\0', '\0', '\0'},                                                       \
     0}
#define LOG_IS_SENTINAL(hl) ((hl).marker[0] == '\0')
#define LOG_MARKERS         {'^', '~', '<'}

ODBUTIL_PUBLIC_API int
log_excerpt(const char* source, const struct log_highlight* highlights);

static inline int
log_excerpt_1(
    const char*      source,
    struct utf8_span location,
    struct utf8_view annotation,
    char             group)
{
    struct utf8_view     ins = empty_utf8_view();
    struct log_highlight inst[]
        = {{ins, annotation, location, LOG_HIGHLIGHT, LOG_MARKERS, group},
           LOG_HIGHLIGHT_SENTINAL};
    return log_excerpt(source, inst);
}

static inline int
log_excerpt_2(
    const char*      source,
    struct utf8_span loc1,
    struct utf8_span loc2,
    struct utf8_view annotation1,
    struct utf8_view annotation2,
    char             group1,
    char             group2)
{
    struct utf8_view     ins = empty_utf8_view();
    struct log_highlight hl[]
        = {{ins, annotation1, loc1, LOG_HIGHLIGHT, LOG_MARKERS, group1},
           {ins, annotation2, loc2, LOG_HIGHLIGHT, LOG_MARKERS, group2},
           LOG_HIGHLIGHT_SENTINAL};
    return log_excerpt(source, hl);
}

static inline int
log_excerpt_binop(
    const char*      source,
    struct utf8_span lhs,
    struct utf8_span op,
    struct utf8_span rhs,
    struct utf8_view lhs_text,
    struct utf8_view rhs_text)
{
    struct utf8_view     ins = empty_utf8_view();
    struct log_highlight hl[]
        = {{ins, lhs_text, lhs, LOG_HIGHLIGHT, {'>', '~', '~'}, 0},
           {ins, empty_utf8_view(), op, LOG_HIGHLIGHT, {'^', '^', '^'}, 2},
           {ins, rhs_text, rhs, LOG_HIGHLIGHT, {'~', '~', '<'}, 1},
           LOG_HIGHLIGHT_SENTINAL};
    if (lhs.len == 1)
        hl[0].marker[0] = '^';
    if (rhs.len == 1)
        hl[2].marker[0] = '^';
    return log_excerpt(source, hl);
}

int
err_assignment_incompatible_types(
    const struct ast* ast,
    ast_id            ass,
    ast_id            first_occurrence,
    struct ospathc    filename,
    const char*       source);
int
err_binop_incompatible_types(
    const struct ast* ast,
    ast_id            source_node,
    ast_id            op,
    struct ospathc    filename,
    const char*       source);
int
err_binop_pow_incompatible_base_type(
    const struct ast* ast,
    ast_id            op,
    union type        base_type,
    union type        target_type,
    struct ospathc    filename,
    const char*       source);
int
err_binop_pow_incompatible_exponent_type(
    const struct ast* ast,
    ast_id            op,
    union type        exp_type,
    union type        target_type,
    struct ospathc    filename,
    const char*       source);
int
err_boolean_invalid_evaluation(
    const struct ast* ast,
    ast_id            expr,
    struct ospathc    filename,
    const char*       source);
int
err_cast_incompatible_types(
    const struct ast* ast,
    ast_id            cast,
    struct ospathc    filename,
    const char*       source);
int
err_func_call_incompatible_types(
    const struct ast* ast,
    ast_id            arg,
    ast_id            param,
    int               arg_num,
    struct ospathc    filename,
    const char*       source);
int
err_func_redefinition(
    const struct ast* func_ast,
    ast_id            func,
    struct ospathc    filename,
    const char*       source,
    const struct ast* first_ast,
    ast_id            first_func,
    struct ospathc    first_filename,
    const char*       first_source);
int
err_func_return_incompatible_types(
    const struct ast* ast,
    ast_id            exit,
    ast_id            func,
    struct ospathc    filename,
    const char*       source);
int
err_func_missing_return_value(
    const struct ast* ast,
    ast_id            func,
    struct utf8_span  ret_loc,
    struct ospathc    filename,
    const char*       source);
int
err_loop_duplicate_name(
    const struct ast* ast,
    struct utf8_span  inner_name,
    struct utf8_span  outer_name,
    struct ospathc    filename,
    const char*       source);
int
err_loop_cont(
    const struct ast* ast,
    ast_id            cont,
    ast_id            first_loop,
    struct ospathc    filename,
    const char*       source);
int
err_loop_exit_not_inside_loop(
    const struct ast* ast,
    ast_id            exit,
    struct ospathc    filename,
    const char*       source);
int
err_loop_exit_unknown_name(
    const struct ast* ast,
    ast_id            exit,
    ast_id            first_loop,
    struct ospathc    filename,
    const char*       source);
int
err_loop_for_unknown_direction(
    const struct ast* ast,
    ast_id            begin,
    ast_id            end,
    ast_id            step,
    struct ospathc    filename,
    const char*       source);
int
err_param_redeclaration(
    const struct ast* ast,
    struct utf8_span  name,
    ast_id            first_occurrence,
    struct ospathc    filename,
    const char*       source);
int
err_select_incompatible_types(
    const struct ast* ast,
    ast_id            select,
    ast_id            case_,
    struct ospathc    filename,
    const char*       source);
int
err_select_duplicate_default(
    const struct ast* ast,
    ast_id            default_case,
    ast_id            first_default_case,
    struct ospathc    filename,
    const char*       source);
int
err_unterminated_remark(
    struct utf8_span location, struct ospathc filename, const char* source);
int
err_var_decl_init_incompatible_types(
    const struct ast* ast,
    ast_id            var_decl,
    struct ospathc    filename,
    const char*       source);
int
err_var_decl_redeclaration(
    const struct ast* ast,
    struct utf8_span  name,
    struct ospathc    filename,
    const char*       source,
    const struct ast* first_ast,
    ast_id            first_occurrence,
    struct ospathc    first_filename,
    const char*       first_source);
int
err_udt_decl_redeclaration(
    const struct ast* ast,
    struct utf8_span  name,
    struct ospathc    filename,
    const char*       source,
    const struct ast* first_ast,
    ast_id            first_occurrence, /* identifier, func_poly or func1 */
    struct ospathc    first_filename,
    const char*       first_source);
int
err_udt_not_found(
    const struct ast* ast,
    struct utf8_span  name,
    struct ospathc    filename,
    const char*       source);
int
err_udt_member_not_found(
    const struct ast* ast,
    struct utf8_span  name,
    struct ospathc    filename,
    const char*       source);
int
err_udt_is_not_udt(
    const struct ast* ast,
    struct utf8_span  name,
    struct ospathc    filename,
    const char*       source);

void
warn_assignment_implicit_conversion(
    const struct ast* ast,
    ast_id            ass,
    ast_id            first_occurrence,
    struct ospathc    filename,
    const char*       source);
void
warn_assignment_truncation(
    const struct ast* ast,
    ast_id            ass,
    ast_id            first_occurrence,
    struct ospathc    filename,
    const char*       source);
void
warn_binop_implicit_conversion(
    const struct ast* ast,
    ast_id            op,
    ast_id            source_node,
    ast_id            target_node,
    struct ospathc    filename,
    const char*       source);
void
warn_binop_truncation(
    const struct ast* ast,
    ast_id            op,
    ast_id            source_node,
    ast_id            target_node,
    struct ospathc    filename,
    const char*       source);
void
warn_binop_pow_base_implicit_conversion(
    const struct ast* ast,
    ast_id            op,
    union type        base_type,
    union type        target_type,
    struct ospathc    filename,
    const char*       source);
void
warn_binop_pow_base_truncation(
    const struct ast* ast,
    ast_id            op,
    union type        base_type,
    union type        target_type,
    struct ospathc    filename,
    const char*       source);
void
warn_binop_pow_exponent_implicit_conversion(
    const struct ast* ast,
    ast_id            op,
    union type        exp_type,
    union type        target_type,
    struct ospathc    filename,
    const char*       source);
void
warn_binop_pow_exponent_truncation(
    const struct ast* ast,
    ast_id            op,
    union type        exp_type,
    union type        target_type,
    struct ospathc    filename,
    const char*       source);
void
warn_boolean_implicit_evaluation(
    const struct ast* ast,
    ast_id            expr,
    struct ospathc    filename,
    const char*       source);
void
warn_cmd_return_value_ignored(
    const struct ast* ast,
    ast_id            cmd,
    struct ospathc    filename,
    const char*       source);
void
warn_func_call_implicit_conversion(
    const struct ast* ast,
    ast_id            arg,
    ast_id            param,
    int               arg_num,
    struct ospathc    filename,
    const char*       source);
void
warn_func_call_return_value_ignored(
    const struct ast* ast,
    ast_id            cmd,
    struct ospathc    filename,
    const char*       source);
void
warn_func_call_truncation(
    const struct ast* ast,
    ast_id            arg,
    ast_id            param,
    int               arg_num,
    struct ospathc    filename,
    const char*       source);
void
warn_func_return_implicit_conversion(
    const struct ast* ast,
    ast_id            exit,
    ast_id            func,
    struct ospathc    filename,
    const char*       source);
void
warn_func_return_truncation(
    const struct ast* ast,
    ast_id            exit,
    ast_id            func,
    struct ospathc    filename,
    const char*       source);
void
warn_loop_exit_ambiguous_name(
    const struct ast* ast,
    ast_id            exit,
    struct utf8_span  name,
    struct utf8_span  outer_name,
    struct ospathc    filename,
    const char*       source);
void
warn_loop_for_default_step_may_be_incorrect(
    const struct ast* ast,
    ast_id            begin,
    ast_id            end,
    struct ospathc    filename,
    const char*       source);
void
warn_loop_for_wrong_direction(
    const struct ast* ast,
    ast_id            begin,
    ast_id            end,
    ast_id            step,
    struct ospathc    filename,
    const char*       source);
void
warn_loop_for_wrong_direction_no_step(
    const struct ast* ast,
    ast_id            begin,
    ast_id            end,
    struct ospathc    filename,
    const char*       source);
void
warn_loop_for_incorrect_next(
    struct ast*    ast,
    ast_id         next,
    ast_id         loop_var,
    struct ospathc filename,
    const char*    source);
void
warn_select_implicit_conversion(
    const struct ast* ast,
    ast_id            select,
    ast_id            case_,
    struct ospathc    filename,
    const char*       source);
void
warn_select_truncation(
    const struct ast* ast,
    ast_id            select,
    ast_id            case_,
    struct ospathc    filename,
    const char*       source);
void
warn_var_decl_implicit_conversion(
    const struct ast* ast,
    ast_id            var_decl,
    struct ospathc    filename,
    const char*       source);
void
warn_var_decl_truncation(
    const struct ast* ast,
    ast_id            var_decl,
    struct ospathc    filename,
    const char*       source);
