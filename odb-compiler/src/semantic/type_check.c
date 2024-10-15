#include "./type_check.h"
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/config.h"
#include "odb-compiler/parser/db_source.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-compiler/semantic/symbol_table.h"
#include "odb-compiler/semantic/type.h"
#include "odb-util/config.h"
#include "odb-util/hash.h"
#include "odb-util/hm.h"
#include "odb-util/log.h"
#include "odb-util/mutex.h"
#include "odb-util/utf8.h"
#include "odb-util/vec.h"
#include <assert.h>

struct span_scope
{
    struct utf8_span span;
    int16_t          scope;
};
struct view_scope
{
    struct utf8_view view;
    int16_t          scope;
};
struct type_origin
{
    enum type type;
    ast_id    original_declaration;
};

VEC_DECLARE_API(static, spanlist, struct span_scope, 32)
VEC_DEFINE_API(spanlist, struct span_scope, 32)

/* The "typemap" is used to track the types of variables. When a variable first
 * appears, it is inserted into the typemap and its type is determined based on
 * the context surrounding it. If the variable is later referenced, then the
 * type is extracted from the typemap.
 *
 * The "text" field references the source text. The spanlist contains
 * utf8_span's that index into the source code. Because it's possible to have
 * the same variable name in a different scope, the key also contains the
 * current scope (0=global, 1, 2, 3, ... = nesting) such that the same variable
 * name hashes to a different value if it is in a different scope.
 */
struct typemap_kvs
{
    const char*         text;
    struct spanlist*    keys;
    struct type_origin* values;
};

static hash32
typemap_kvs_hash(struct view_scope key)
{
    return hash32_jenkins_oaat(key.view.data + key.view.off, key.view.len)
           + key.scope;
}
static int
typemap_kvs_alloc(
    struct typemap_kvs* kvs, struct typemap_kvs* old_kvs, int32_t capacity)
{
    kvs->text = NULL;
    spanlist_init(&kvs->keys);
    if (spanlist_resize(&kvs->keys, capacity) != 0)
        return -1;

    if ((kvs->values = mem_alloc(sizeof(*kvs->values) * capacity)) == NULL)
    {
        spanlist_deinit(kvs->keys);
        return log_oom(sizeof(enum type) * capacity, "typemap_kvs_alloc()");
    }

    return 0;
}
static void
typemap_kvs_free_old(struct typemap_kvs* kvs)
{
    mem_free(kvs->values);
    spanlist_deinit(kvs->keys);
}
static void
typemap_kvs_free(struct typemap_kvs* kvs)
{
    mem_free(kvs->values);
    spanlist_deinit(kvs->keys);
}
static struct view_scope
typemap_kvs_get_key(const struct typemap_kvs* kvs, int32_t slot)
{
    ODBUTIL_DEBUG_ASSERT(kvs->text != NULL, (void)0);
    struct span_scope span_scope = kvs->keys->data[slot];
    struct utf8_view  view = utf8_span_view(kvs->text, span_scope.span);
    struct view_scope view_scope = {view, span_scope.scope};
    return view_scope;
}
static int
typemap_kvs_set_key(
    struct typemap_kvs* kvs, int32_t slot, struct view_scope key)
{
    ODBUTIL_DEBUG_ASSERT(
        kvs->text == NULL || kvs->text == key.view.data, (void)0);

    kvs->text = key.view.data;
    struct utf8_span  span = utf8_view_span(kvs->text, key.view);
    struct span_scope span_scope = {span, key.scope};
    kvs->keys->data[slot] = span_scope;

    return 0;
}
static int
typemap_kvs_keys_equal(struct view_scope k1, struct view_scope k2)
{
    return k1.scope == k2.scope && utf8_equal(k1.view, k2.view);
}
static struct type_origin*
typemap_kvs_get_value(const struct typemap_kvs* kvs, int32_t slot)
{
    return &kvs->values[slot];
}
static void
typemap_kvs_set_value(
    struct typemap_kvs* kvs, int32_t slot, struct type_origin* value)
{
    kvs->values[slot] = *value;
}

HM_DECLARE_API_FULL(
    static,
    typemap,
    hash32,
    struct view_scope,
    struct type_origin,
    32,
    struct typemap_kvs)
HM_DEFINE_API_FULL(
    typemap,
    hash32,
    struct view_scope,
    struct type_origin,
    32,
    typemap_kvs_hash,
    typemap_kvs_alloc,
    typemap_kvs_free_old,
    typemap_kvs_free,
    typemap_kvs_get_key,
    typemap_kvs_set_key,
    typemap_kvs_keys_equal,
    typemap_kvs_get_value,
    typemap_kvs_set_value,
    32,
    70)

static void
typemap_clear_all_with_scope(struct typemap* hm, int16_t scope)
{
    int slot;
    for (slot = 0; slot != typemap_capacity(hm); ++slot)
    {
        if (hm->hashes[slot] == HM_SLOT_UNUSED
            || hm->hashes[slot] == HM_SLOT_RIP)
        {
            continue;
        }

        if (vec_get(hm->kvs.keys, slot)->scope == scope)
        {
            hm->hashes[slot] = HM_SLOT_RIP;
            hm->count--;
        }
    }
}

struct ctx
{
    struct ast**               tus;
    struct mutex**             tu_mutexes;
    const struct utf8*         filenames;
    const struct db_source*    sources;
    const struct cmd_list*     cmds;
    const struct symbol_table* symbols;
    struct typemap*            typemap;
    const int                  tu_id;
};

static struct ctx
create_context_for_tu(const struct ctx* ctx, int tu_id)
{
    struct ctx new_ctx
        = {ctx->tus,
           ctx->tu_mutexes,
           ctx->filenames,
           ctx->sources,
           ctx->cmds,
           ctx->symbols,
           ctx->typemap,
           tu_id};
    return new_ctx;
}

static int
cast_expr_to_boolean(
    struct ast** astp,
    ast_id       n,
    ast_id       parent,
    const char*  filename,
    const char*  source)
{
    struct ast* ast = *astp;
    if (ast->nodes[n].info.type_info == TYPE_BOOL)
        return 0;

    switch (type_convert(ast->nodes[n].info.type_info, TYPE_BOOL))
    {
        case TC_TRUENESS: {
            int                  gutter;
            struct utf8_span     expr_loc = ast->nodes[n].info.location;
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
                ast->nodes[n].info.location,
                "Implicit evaluation of {emph1:%s} as a boolean expression.\n",
                type_to_db_name(ast->nodes[n].info.type_info));
            gutter = log_excerpt_1(
                source,
                ast->nodes[n].info.location,
                type_to_db_name(ast->nodes[n].info.type_info));

            log_excerpt_help(
                gutter, "You can make it explicit by changing it to:\n");
            switch (ast->nodes[n].info.type_info)
            {
                case TYPE_INVALID:
                case TYPE_VOID:
                case TYPE_BOOL: ODBUTIL_DEBUG_ASSERT(0, (void)0); break;

                case TYPE_I64: /* fallthrough */
                case TYPE_U32: /* fallthrough */
                case TYPE_I32: /* fallthrough */
                case TYPE_U16: /* fallthrough */
                case TYPE_U8: log_excerpt(source, hl_int); break;
                case TYPE_F32: log_excerpt(source, hl_float); break;
                case TYPE_F64: log_excerpt(source, hl_double); break;
                case TYPE_STRING: log_excerpt(source, hl_string); break;

                case TYPE_ARRAY: break;

                case TYPE_LABEL:
                case TYPE_DABEL:
                case TYPE_ANY:
                case TYPE_USER_DEFINED_VAR_PTR:
                    ODBUTIL_DEBUG_ASSERT(0, (void)0);
                    break;
            }
        }
            /* fallthrough */
        case TC_ALLOW: {
            ast_id cast
                = ast_cast(astp, n, TYPE_BOOL, ast->nodes[n].info.location);
            ast = *astp;
            if (cast < 0)
                return TYPE_INVALID;

            /* Insert cast in between */
            if (ast->nodes[parent].base.left == n)
                ast->nodes[parent].base.left = cast;
            if (ast->nodes[parent].base.right == n)
                ast->nodes[parent].base.right = cast;

            return 0;
        }

        case TC_DISALLOW:
            log_flc_err(
                filename,
                source,
                ast->nodes[n].info.location,
                "Cannot evaluate {emph1:%s} as a boolean expression.\n",
                type_to_db_name(ast->nodes[n].info.type_info));
            log_excerpt_1(
                source,
                ast->nodes[n].info.location,
                type_to_db_name(ast->nodes[n].info.type_info));
            break;

        case TC_SIGN_CHANGE:
        case TC_TRUNCATE:
        case TC_INT_TO_FLOAT:
        case TC_BOOL_PROMOTION: ODBUTIL_DEBUG_ASSERT(0, (void)0); break;
    }

    return -1;
}

static ast_id
instantiate_func(
    /* TU in which the templated function exists */
    struct ctx* func_ctx,
    ast_id      func_template,
    /* TU in which the call is being made (contains the arglist) */
    struct ctx* call_ctx,
    ast_id      call)
{
    ast_id func, template_block, decl, arglist, paramlist, pl_entry, arg_entry;
    struct ast** func_astp = &func_ctx->tus[func_ctx->tu_id];
    struct ast** call_astp = &call_ctx->tus[call_ctx->tu_id];

    ODBUTIL_DEBUG_ASSERT(
        (*func_astp)->nodes[func_template].info.node_type == AST_FUNC_TEMPLATE,
        log_semantic_err(
            "type: %d\n", (*func_astp)->nodes[func_template].info.node_type));

    template_block = ast_find_parent(*func_astp, func_template);
    ODBUTIL_DEBUG_ASSERT(
        (*func_astp)->nodes[template_block].info.node_type == AST_BLOCK,
        log_semantic_err(
            "type: %d\n", (*func_astp)->nodes[template_block].info.node_type));

    func = ast_dup_subtree(func_astp, func_template);
    if (func < 0)
        return TYPE_INVALID;

    /* Insert new function into AST after the template */
    if (ast_block_append_stmt(
            func_astp,
            template_block,
            func,
            (*func_astp)->nodes[func].info.location)
        < 0)
    {
        return TYPE_INVALID;
    }

    /* The arguments passed to the function determine the parameter types. We
     * copy them over here. Some function templates are "partial" templates,
     * i.e. one parameter carries type information but another does not. In
     * these cases we must insert casts to the correct type. */
    decl = (*func_astp)->nodes[func].func.decl;
    ODBUTIL_DEBUG_ASSERT(
        (*call_astp)->nodes[call].info.node_type == AST_FUNC_CALL,
        log_semantic_err(
            "type: %d\n", (*call_astp)->nodes[call].info.node_type));
    arglist = (*call_astp)->nodes[call].func_call.arglist;
    paramlist = (*func_astp)->nodes[decl].func_decl.paramlist;
    for (pl_entry = paramlist, arg_entry = arglist;
         pl_entry > -1 && arg_entry > -1;
         pl_entry = (*func_astp)->nodes[pl_entry].paramlist.next,
        arg_entry = (*func_astp)->nodes[arg_entry].arglist.next)
    {
        ast_id    param = (*func_astp)->nodes[pl_entry].paramlist.identifier;
        ast_id    arg = (*call_astp)->nodes[arg_entry].arglist.expr;
        enum type param_type = (*func_astp)->nodes[param].info.type_info;
        enum type arg_type = (*call_astp)->nodes[arg].info.type_info;

        if ((*func_astp)->nodes[param].identifier.explicit_type == TYPE_INVALID)
        {
            /* This parameter has not been declared with an explicit type,
             * therefore it takes the type of the argument being passed in */
            (*func_astp)->nodes[param].identifier.explicit_type = arg_type;
        }
        else if (param_type != arg_type)
        {
            /* The parameter has been declared with an explicit type, and the
             * argument being passed in has a different type. Need to insert a
             * cast */
            /* TODO: Verify the cast is valid */
            ast_id cast = ast_cast(
                call_astp,
                arg,
                param_type,
                (*call_astp)->nodes[arg].info.location);
            if (cast < -1)
                return -1;
            (*call_astp)->nodes[arg_entry].arglist.expr = cast;
        }
    }
    if (arg_entry > -1 || pl_entry > -1)
    {
        int         gutter;
        const char* filename = utf8_cstr(call_ctx->filenames[call_ctx->tu_id]);
        const char* source = call_ctx->sources[call_ctx->tu_id].text.data;
        log_flc_err(
            filename,
            source,
            (*call_astp)->nodes[call].info.location,
            arg_entry > -1 ? "Too many arguments to function call.\n"
                           : "Too few arguments to function call.\n");
        gutter = log_excerpt_1(
            source, (*call_astp)->nodes[call].info.location, "");
        log_excerpt_note(gutter, "Function has the following signature:\n");
        log_excerpt_1(source, (*func_astp)->nodes[decl].info.location, "");
        return -1;
    }

    /* Since the function parameter types are all known now, this is no longer a
     * function template */
    (*func_astp)->nodes[func].info.node_type = AST_FUNC;

    return func;
}

static enum type
resolve_node_type(struct ctx* ctx, ast_id n, int16_t scope)
{
    struct ast** astp = &ctx->tus[ctx->tu_id];
    const char*  filename = utf8_cstr(ctx->filenames[ctx->tu_id]);
    const char*  source = ctx->sources[ctx->tu_id].text.data;
    ODBUTIL_DEBUG_ASSERT(n > -1, (void)0);

    /* Ensure we aren't resolving nodes more than once */
    if ((*astp)->nodes[n].info.type_info != TYPE_INVALID)
        return (*astp)->nodes[n].info.type_info;

    /* Nodes that don't return a value should be marked as TYPE_VOID */
    switch ((*astp)->nodes[n].info.node_type)
    {
        case AST_GC: ODBUTIL_DEBUG_ASSERT(0, (void)0); break;
        case AST_BLOCK: {
            ast_id block;
            for (block = n; block > -1;
                 block = (*astp)->nodes[block].block.next)
            {
                ast_id    stmt = (*astp)->nodes[block].block.stmt;
                enum type type = resolve_node_type(ctx, stmt, scope);
                if (type == TYPE_INVALID)
                    return TYPE_INVALID;

                /* Blocks are not expressions */
                (*astp)->nodes[block].info.type_info = TYPE_VOID;
            }
            return TYPE_VOID; /* Blocks are not expressions */
        }

        case AST_END: return (*astp)->nodes[n].info.type_info = TYPE_VOID;

        case AST_ARGLIST: {
            ast_id arg;
            for (arg = n; arg > -1; arg = (*astp)->nodes[arg].arglist.next)
            {
                ast_id    expr = (*astp)->nodes[arg].arglist.expr;
                enum type type = resolve_node_type(ctx, expr, scope);
                if (type == TYPE_INVALID)
                    return TYPE_INVALID;

                /* Argument lists are not expressions */
                (*astp)->nodes[arg].info.type_info = TYPE_VOID;
            }
            return TYPE_VOID; /* Argument lists are not expressions */
        }

        case AST_PARAMLIST:
            ODBUTIL_DEBUG_ASSERT(
                0,
                log_semantic_err(
                    "Parameter lists are handled directly in AST_FUNC\n"));
            return TYPE_INVALID;

        case AST_ASSIGNMENT: {
            ast_id    lhs = (*astp)->nodes[n].assignment.lvalue;
            ast_id    rhs = (*astp)->nodes[n].assignment.expr;
            enum type rhs_type = resolve_node_type(ctx, rhs, scope);
            if (rhs_type == TYPE_INVALID)
                return TYPE_INVALID;

            if ((*astp)->nodes[lhs].info.node_type == AST_IDENTIFIER)
            {
                struct type_origin* lhs_type;
                struct utf8_view    lhs_name = utf8_span_view(
                    source, (*astp)->nodes[lhs].identifier.name);
                struct view_scope lhs_name_scope = {lhs_name, scope};

                enum hm_status lhs_insertion = typemap_emplace_or_get(
                    &ctx->typemap, lhs_name_scope, &lhs_type);
                switch (lhs_insertion)
                {
                    case HM_OOM: return TYPE_INVALID;
                    case HM_EXISTS: break;
                    case HM_NEW: {
                        lhs_type->original_declaration = lhs;

                        /* Prefer the explicit type ("AS TYPE"). This is set by
                         * the parser */
                        lhs_type->type
                            = (*astp)->nodes[lhs].identifier.explicit_type;
                        if (lhs_type->type != TYPE_INVALID)
                            break;

                        /* Otherwise use annotated type */
                        lhs_type->type = annotation_to_type(
                            (*astp)->nodes[lhs].identifier.annotation);
                        break;
                    }
                }
                (*astp)->nodes[lhs].info.type_info = lhs_type->type;

                /* May need to insert a cast from rhs to lhs */
                if (rhs_type != lhs_type->type)
                {
                    int    gutter;
                    ast_id orig_node = lhs_type->original_declaration;
                    ODBUTIL_DEBUG_ASSERT(
                        (*astp)->nodes[orig_node].info.node_type
                            == AST_IDENTIFIER,
                        log_semantic_err(
                            "type: %d\n",
                            (*astp)->nodes[orig_node].info.node_type));
                    struct utf8_span orig_name
                        = (*astp)->nodes[orig_node].identifier.name;
                    struct utf8_span orig_loc
                        = (*astp)->nodes[orig_node].info.location;

                    ast_id cast = ast_cast(
                        astp,
                        rhs,
                        lhs_type->type,
                        (*astp)->nodes[rhs].info.location);
                    if (cast < -1)
                        return TYPE_INVALID;
                    (*astp)->nodes[n].assignment.expr = cast;

                    switch (type_convert(rhs_type, lhs_type->type))
                    {
                        case TC_ALLOW: break;
                        case TC_DISALLOW:
                            log_flc_err(
                                filename,
                                source,
                                (*astp)->nodes[rhs].info.location,
                                "Cannot assign {emph2:%s} to {emph1:%s}. Types "
                                "are incompatible.\n",
                                type_to_db_name(rhs_type),
                                type_to_db_name(lhs_type->type));
                            gutter = log_excerpt_binop(
                                source,
                                (*astp)->nodes[lhs].info.location,
                                (*astp)->nodes[n].assignment.op_location,
                                (*astp)->nodes[rhs].info.location,
                                type_to_db_name(lhs_type->type),
                                type_to_db_name(rhs_type));
                            log_excerpt_note(
                                gutter,
                                "{emph1:%.*s} was previously declared as "
                                "{emph1:%s} at ",
                                orig_name.len,
                                source + orig_name.off,
                                type_to_db_name(lhs_type->type));
                            log_flc("", filename, source, orig_loc, "\n");
                            log_excerpt_1(
                                source,
                                orig_loc,
                                type_to_db_name(lhs_type->type));
                            return TYPE_INVALID;

                        case TC_SIGN_CHANGE:
                        case TC_TRUENESS:
                        case TC_INT_TO_FLOAT:
                        case TC_BOOL_PROMOTION:
                            if (lhs_insertion == HM_NEW)
                            {
                                /* clang-format off */
                                const char* as_type = type_to_db_name(rhs_type);
                                char ann[2] = {type_to_annotation(rhs_type), '\0'};
                                struct utf8_span lhs_loc = (*astp)->nodes[lhs].info.location;
                                utf8_idx lhs_end = lhs_loc.off + lhs_loc.len;
                                struct log_highlight hl_annotation[] = {
                                    {ann, "", {lhs_end, 1}, LOG_INSERT, LOG_MARKERS, 0},
                                    LOG_HIGHLIGHT_SENTINAL
                                };
                                struct log_highlight hl_as_type[] = {
                                    {" AS ", "", {lhs_end, 4}, LOG_INSERT, {'^', '~', '~'}, 0},
                                    {as_type, "", {lhs_end, strlen(as_type)}, LOG_INSERT, {'~', '~', '<'}, 0},
                                    LOG_HIGHLIGHT_SENTINAL
                                };
                                /* clang-format on */
                                log_flc_warn(
                                    filename,
                                    source,
                                    (*astp)->nodes[rhs].info.location,
                                    "Implicit conversion from {emph2:%s} to "
                                    "{emph1:%s} in variable initialization.\n",
                                    type_to_db_name(rhs_type),
                                    type_to_db_name(lhs_type->type));
                                gutter = log_excerpt_binop(
                                    source,
                                    (*astp)->nodes[lhs].info.location,
                                    (*astp)->nodes[n].assignment.op_location,
                                    (*astp)->nodes[rhs].info.location,
                                    type_to_db_name(lhs_type->type),
                                    type_to_db_name(rhs_type));
                                if (ann[0] != TA_NONE)
                                {
                                    log_excerpt_help(
                                        gutter, "Annotate the variable:\n");
                                    log_excerpt(source, hl_annotation);
                                }
                                log_excerpt_help(
                                    gutter,
                                    "%sxplicitly declare the type of the "
                                    "variable:\n",
                                    ann[0] != TA_NONE ? "Or e" : "E");
                                log_excerpt(source, hl_as_type);
                            }
                            else
                            {
                                log_flc_warn(
                                    filename,
                                    source,
                                    (*astp)->nodes[rhs].info.location,
                                    "Implicit conversion from {emph2:%s} to "
                                    "{emph1:%s} in assignment.\n",
                                    type_to_db_name(rhs_type),
                                    type_to_db_name(lhs_type->type));
                                gutter = log_excerpt_binop(
                                    source,
                                    (*astp)->nodes[lhs].info.location,
                                    (*astp)->nodes[n].assignment.op_location,
                                    (*astp)->nodes[rhs].info.location,
                                    type_to_db_name(lhs_type->type),
                                    type_to_db_name(rhs_type));
                                log_excerpt_note(
                                    gutter,
                                    "{emph1:%.*s} was previously declared as "
                                    "{emph1:%s} at ",
                                    orig_name.len,
                                    source + orig_name.off,
                                    type_to_db_name(lhs_type->type));
                                log_flc("", filename, source, orig_loc, "\n");
                                log_excerpt_1(
                                    source,
                                    orig_loc,
                                    type_to_db_name(lhs_type->type));
                            }
                            break;

                        case TC_TRUNCATE:
                            log_flc_warn(
                                filename,
                                source,
                                (*astp)->nodes[rhs].info.location,
                                "Value is truncated when converting from "
                                "{emph2:%s} to {emph1:%s} in assignment.\n",
                                type_to_db_name(rhs_type),
                                type_to_db_name(lhs_type->type));
                            gutter = log_excerpt_binop(
                                source,
                                (*astp)->nodes[lhs].info.location,
                                (*astp)->nodes[n].assignment.op_location,
                                (*astp)->nodes[rhs].info.location,
                                type_to_db_name(lhs_type->type),
                                type_to_db_name(rhs_type));
                            log_excerpt_note(
                                gutter,
                                "{emph1:%.*s} was previously declared as "
                                "{emph1:%s} at ",
                                orig_name.len,
                                source + orig_name.off,
                                type_to_db_name(lhs_type->type));
                            log_flc("", filename, source, orig_loc, "\n");
                            log_excerpt_1(
                                source,
                                orig_loc,
                                type_to_db_name(lhs_type->type));
                            break;
                    }
                }
            }

            /* Assignments are not expressions, thus they do not evaluate to a
             * type */
            return (*astp)->nodes[n].info.type_info = TYPE_VOID;
        }

        case AST_COMMAND: {
            ast_id arglist = (*astp)->nodes[n].cmd.arglist;
            cmd_id cmd_id = (*astp)->nodes[n].cmd.id;
            for (; arglist > -1; arglist = (*astp)->nodes[arglist].arglist.next)
            {
                ast_id    expr = (*astp)->nodes[arglist].arglist.expr;
                enum type arg_type = resolve_node_type(ctx, expr, scope);
                if (arg_type == TYPE_INVALID)
                    return TYPE_INVALID;
                /* Argument lists are not expressions */
                (*astp)->nodes[arglist].info.type_info = TYPE_VOID;
            }

            return (*astp)->nodes[n].cmd.info.type_info
                   = ctx->cmds->return_types->data[cmd_id];
        }

        case AST_IDENTIFIER: {
            struct type_origin* type_origin;
            struct utf8_view    name
                = utf8_span_view(source, (*astp)->nodes[n].identifier.name);
            struct view_scope view_scope = {name, scope};
            switch (
                typemap_emplace_or_get(&ctx->typemap, view_scope, &type_origin))
            {
                case HM_NEW: {
                    struct utf8_span loc;
                    ast_id           init_lit;

                    type_origin->original_declaration = n;

                    /* Prefer the explicit type ("AS TYPE"). This is set by the
                     * parser */
                    type_origin->type
                        = (*astp)->nodes[n].identifier.explicit_type;

                    /* Otherwise use annotated type */
                    if (type_origin->type == TYPE_INVALID)
                        type_origin->type = annotation_to_type(
                            (*astp)->nodes[n].identifier.annotation);

                    /* Determine initializer expression based on the deduced
                     * type */
                    loc = (*astp)->nodes[n].info.location;
                    switch (type_origin->type)
                    {
                        case TYPE_BOOL:
                            init_lit = ast_boolean_literal(astp, 0, loc);
                            break;
                        case TYPE_I64:
                            init_lit = ast_double_integer_literal(astp, 0, loc);
                            break;
                        case TYPE_U32:
                            init_lit = ast_dword_literal(astp, 0, loc);
                            break;
                        case TYPE_I32:
                            init_lit = ast_integer_literal(astp, 0, loc);
                            break;
                        case TYPE_U16:
                            init_lit = ast_word_literal(astp, 0, loc);
                            break;
                        case TYPE_U8:
                            init_lit = ast_byte_literal(astp, 0, loc);
                            break;
                        case TYPE_F32:
                            init_lit = ast_float_literal(astp, 0, loc);
                            break;
                        case TYPE_F64:
                            init_lit = ast_double_literal(astp, 0, loc);
                            break;
                        case TYPE_STRING:
                            init_lit = ast_string_literal(
                                astp, empty_utf8_span(), loc);
                            break;

                        case TYPE_INVALID:
                        case TYPE_VOID:
                        case TYPE_ARRAY:
                        case TYPE_LABEL:
                        case TYPE_DABEL:
                        case TYPE_ANY:
                        case TYPE_USER_DEFINED_VAR_PTR:
                            ODBUTIL_DEBUG_ASSERT(0, (void)0);
                            goto not_yet_implemented;
                    }

                    /* The initializer is inserted into the AST differently
                     * depending on the context. There are two cases.
                     *
                     * 1. If the variable is a declaration, i.e. it is its own
                     *    statement within a block, then we must remove the
                     *    declaration from the AST and replace it with the
                     *    initializer (which is just an assignemnt statement).
                     *    In code:
                     *      a AS INTEGER
                     *    becomes:
                     *      a AS INTEGER = 0
                     *
                     * 2. If the variable is an expression, e.g. it is being
                     *    referenced on the RHS of an assignment, or appeared in
                     *    a function call, then we must insert an initializer
                     *    somewhere before that statement. The best place to
                     *    insert it is at the beginning of the current scope.
                     *    In code:
                     *      print a
                     *    becomes:
                     *      a = 0
                     *      print a
                     */

                    /* TODO: Global variables are not yet supported */

                    /* Currently, if the identifier's explicit_type property is
                     * set, only then can it be an lvalue. This is enforced by
                     * the parser. In all other cases it is an rvalue. */
                    if ((*astp)->nodes[n].identifier.explicit_type
                        != TYPE_INVALID)
                    {
                        /* Case 1: Declaration */
                        ast_id init_ass
                            = ast_assign(astp, n, init_lit, loc, loc);

                        ast_id parent = ast_find_parent(*astp, n);
                        if ((*astp)->nodes[parent].base.left == n)
                            (*astp)->nodes[parent].base.left = init_ass;
                        if ((*astp)->nodes[parent].base.right == n)
                            (*astp)->nodes[parent].base.right = init_ass;

                        (*astp)->nodes[init_lit].info.type_info
                            = type_origin->type;
                        (*astp)->nodes[init_ass].info.type_info = TYPE_VOID;

                        break;
                    }
                    else
                    {
                        /* Case 2: Need to insert an init statement */

                        ast_id init_var = ast_dup_lvalue(astp, n);
                        ast_id init_ass
                            = ast_assign(astp, init_var, init_lit, loc, loc);
                        ast_id init_block = ast_block(astp, init_ass, loc);

                        /* Fill in type info */
                        (*astp)->nodes[init_lit].info.type_info
                            = type_origin->type;
                        (*astp)->nodes[init_var].info.type_info
                            = type_origin->type;
                        (*astp)->nodes[init_ass].info.type_info = TYPE_VOID;
                        (*astp)->nodes[init_block].info.type_info = TYPE_VOID;

                        /* Insert the initializer at the very beginning of the
                         * program */
                        /* XXX: This won't work for variables created on the
                         * stack in function bodies. I don't know if DBP creates
                         * scopes in other constructs such as if-endif blocks.
                         * Need to investigate. */
                        (*astp)->nodes[init_block].block.next = (*astp)->root;
                        (*astp)->root = init_block;
                    }
                    break;
                }

                case HM_EXISTS: break;
                case HM_OOM: return TYPE_INVALID;
            }

            return (*astp)->nodes[n].identifier.info.type_info
                   = type_origin->type;
        }

        case AST_BINOP: {
            ast_id lhs = (*astp)->nodes[n].binop.left;
            ast_id rhs = (*astp)->nodes[n].binop.right;
            if (resolve_node_type(ctx, lhs, scope) < 0)
                return TYPE_INVALID;
            if (resolve_node_type(ctx, rhs, scope) < 0)
                return TYPE_INVALID;

            switch ((*astp)->nodes[n].binop.op)
            {
                case BINOP_ADD:
                case BINOP_SUB:
                case BINOP_MUL:
                case BINOP_DIV:
                case BINOP_MOD:
                    return type_check_binop_symmetric(
                        astp, n, filename, source);

                case BINOP_POW:
                    return type_check_binop_pow(astp, n, filename, source);

                case BINOP_SHIFT_LEFT:
                case BINOP_SHIFT_RIGHT:
                case BINOP_BITWISE_OR:
                case BINOP_BITWISE_AND:
                case BINOP_BITWISE_XOR:
                case BINOP_BITWISE_NOT: break;

                case BINOP_LESS_THAN:
                case BINOP_LESS_EQUAL:
                case BINOP_GREATER_THAN:
                case BINOP_GREATER_EQUAL:
                case BINOP_EQUAL:
                case BINOP_NOT_EQUAL:
                    if (type_check_binop_symmetric(astp, n, filename, source)
                        == TYPE_INVALID)
                    {
                        return TYPE_INVALID;
                    }
                    return (*astp)->nodes[n].binop.info.type_info = TYPE_BOOL;

                case BINOP_LOGICAL_OR:
                case BINOP_LOGICAL_AND:
                case BINOP_LOGICAL_XOR: {
                    if (cast_expr_to_boolean(astp, lhs, n, filename, source)
                        != 0)
                    {
                        return TYPE_INVALID;
                    }
                    if (cast_expr_to_boolean(astp, rhs, n, filename, source)
                        != 0)
                    {
                        return TYPE_INVALID;
                    }

                    return (*astp)->nodes[n].binop.info.type_info = TYPE_BOOL;
                }
                break;
            }
        }
        break;

        case AST_UNOP: break;

        case AST_COND: {
            ast_id expr = (*astp)->nodes[n].cond.expr;
            ast_id cond_branch = (*astp)->nodes[n].cond.cond_branch;

            ODBUTIL_DEBUG_ASSERT(
                (*astp)->nodes[cond_branch].info.node_type == AST_COND_BRANCH,
                log_semantic_err(
                    "type: %d\n", (*astp)->nodes[cond_branch].info.node_type));
            ast_id yes = (*astp)->nodes[cond_branch].cond_branch.yes;
            ast_id no = (*astp)->nodes[cond_branch].cond_branch.no;

            (*astp)->nodes[expr].info.type_info
                = resolve_node_type(ctx, expr, scope);
            if ((*astp)->nodes[expr].info.type_info == TYPE_INVALID)
                return TYPE_INVALID;

            if (yes > -1 && resolve_node_type(ctx, yes, scope) == TYPE_INVALID)
                return TYPE_INVALID;
            if (no > -1 && resolve_node_type(ctx, no, scope) == TYPE_INVALID)
                return TYPE_INVALID;

            /* The expression is always evaluated to a bool. If this is not the
             * case here, then insert a cast */
            if (cast_expr_to_boolean(astp, expr, n, filename, source) != 0)
                return TYPE_INVALID;

            (*astp)->nodes[cond_branch].info.type_info = TYPE_VOID;
            (*astp)->nodes[n].info.type_info = TYPE_VOID;
            return TYPE_VOID;
        }

        case AST_COND_BRANCH: ODBUTIL_DEBUG_ASSERT(0, (void)0); break;

        case AST_LOOP: {
            ast_id body = (*astp)->nodes[n].loop.body;
            ast_id step = (*astp)->nodes[n].loop.post_body;
            ODBUTIL_DEBUG_ASSERT(
                (*astp)->nodes[n].loop.loop_for == -1,
                log_semantic_err(
                    "loop_for: %d\n", (*astp)->nodes[n].loop.loop_for));

            if (body > -1
                && resolve_node_type(ctx, body, scope) == TYPE_INVALID)
                return TYPE_INVALID;

            if (step > -1
                && resolve_node_type(ctx, step, scope) == TYPE_INVALID)
                return TYPE_INVALID;

            return (*astp)->nodes[n].info.type_info = TYPE_VOID;
        }

        case AST_LOOP_FOR: ODBUTIL_DEBUG_ASSERT(0, (void)0); break;
        case AST_LOOP_CONT: {
            ast_id step = (*astp)->nodes[n].cont.step;
            if (step > -1
                && resolve_node_type(ctx, step, scope) == TYPE_INVALID)
                return TYPE_INVALID;

            return (*astp)->nodes[n].info.type_info = TYPE_VOID;
        }

        case AST_LOOP_EXIT: return (*astp)->nodes[n].info.type_info = TYPE_VOID;

        case AST_FUNC_TEMPLATE: {
            /* Function templates cannot be resolved without knowing the input
             * parameters at the callsite. The node is encountered because it is
             * a child of block nodes. It should never be encountered in any
             * other situation. Functions are instantiated in case
             * AST_FUNC_OR_CONTAINER_REF. */
            ODBUTIL_DEBUG_ASSERT(
                (*astp)->nodes[ast_find_parent(*astp, n)].info.node_type
                    == AST_BLOCK,
                (void)0);
            return TYPE_VOID;
        }

        case AST_FUNC: {
            enum type ret_type;
            ast_id    decl, def, paramlist, body, ret, identifier;

            /* Function opens a new scope */
            scope++;

            decl = (*astp)->nodes[n].func.decl;
            def = (*astp)->nodes[n].func.def;
            ODBUTIL_DEBUG_ASSERT(decl > -1, (void)0);
            ODBUTIL_DEBUG_ASSERT(def > -1, (void)0);

            body = (*astp)->nodes[def].func_def.body;
            ret = (*astp)->nodes[def].func_def.retval;

            for (paramlist = (*astp)->nodes[decl].func_decl.paramlist;
                 paramlist > -1;
                 paramlist = (*astp)->nodes[paramlist].paramlist.next)
            {
                struct type_origin* type_origin;
                ast_id param = (*astp)->nodes[paramlist].paramlist.identifier;
                struct utf8_view name = utf8_span_view(
                    source, (*astp)->nodes[param].identifier.name);
                struct view_scope view_scope = {name, scope};

                ODBUTIL_DEBUG_ASSERT(
                    (*astp)->nodes[param].identifier.explicit_type
                        != TYPE_INVALID,
                    log_semantic_err(
                        "AST_FUNC nodes MUST have all of their parameters "
                        "declared with explicit types, otherwise it is is a "
                        "AST_FUNC_TEMPLATE, not a AST_FUNC.\n"));
                (*astp)->nodes[param].info.type_info
                    = (*astp)->nodes[param].identifier.explicit_type;

                switch (typemap_emplace_or_get(
                    &ctx->typemap, view_scope, &type_origin))
                {
                    case HM_NEW:
                        type_origin->original_declaration = param;
                        type_origin->type
                            = (*astp)->nodes[param].info.type_info;
                        break;

                    case HM_EXISTS:
                        ODBUTIL_DEBUG_ASSERT(0, (void)0);
                        /* fallthrough */
                    case HM_OOM: return TYPE_INVALID;
                }

                (*astp)->nodes[paramlist].info.type_info = TYPE_VOID;
            }

            if (body > -1
                && resolve_node_type(ctx, body, scope) == TYPE_INVALID)
            {
                return TYPE_INVALID;
            }

            ret_type = TYPE_VOID;
            if (ret > -1)
            {
                ret_type = resolve_node_type(ctx, ret, scope);
                if (ret_type == TYPE_INVALID)
                    return TYPE_INVALID;
            }

            /* TODO: Type check function's annotation
            ODBUTIL_DEBUG_ASSERT(identifier > -1, (void)0);
            ident_type = annotation_to_type(
                (*astp)->nodes[identifier].identifier.annotation);
            (*astp)->nodes[identifier].info.type_info = ident_type;
            */

            /* TODO: Type check identifier's return type with explicit_type */

            identifier = (*astp)->nodes[decl].func_decl.identifier;
            (*astp)->nodes[identifier].info.type_info = ret_type;
            (*astp)->nodes[decl].info.type_info = ret_type;
            (*astp)->nodes[def].info.type_info = ret_type;
            (*astp)->nodes[n].info.type_info = ret_type;

            /* Close scope again */
            typemap_clear_all_with_scope(ctx->typemap, scope--);

            return ret_type;
        }
        case AST_FUNC_DECL:
        case AST_FUNC_DEF:
            /* Are handled by AST_FUNC */
            ODBUTIL_DEBUG_ASSERT(0, (void)0);
            return TYPE_INVALID;

        case AST_FUNC_EXIT: {
            ast_id ret = (*astp)->nodes[n].func_exit.retval;
            if (ret > -1 && resolve_node_type(ctx, ret, scope) == TYPE_INVALID)
                return TYPE_INVALID;

            return (*astp)->nodes[n].info.type_info = TYPE_VOID;
        }

        case AST_FUNC_OR_CONTAINER_REF: {
            enum type ret_type;
            ast_id    identifier, arglist;

            /* Look up the identifier in the symbol table to figure out if it
             * exists and whether it is a function or a container */
            identifier = (*astp)->nodes[n].func_or_container_ref.identifier;
            struct utf8_view key = utf8_span_view(
                source, (*astp)->nodes[identifier].identifier.name);
            struct symbol_table_entry* entry
                = symbol_table_find(ctx->symbols, key);
            if (entry == NULL)
            {
                log_flc_err(
                    filename,
                    source,
                    (*astp)->nodes[identifier].info.location,
                    "No function with this name exists.\n");
                log_excerpt_1(source, (*astp)->nodes[n].info.location, "");
                return TYPE_INVALID;
            }

            /* At this point we know it is a function call */
            (*astp)->nodes[n].info.node_type = AST_FUNC_CALL;

            /* Resolve types of each argument to the function call */
            arglist = (*astp)->nodes[n].func_or_container_ref.arglist;
            for (; arglist > -1; arglist = (*astp)->nodes[arglist].arglist.next)
            {
                ast_id arg = (*astp)->nodes[arglist].arglist.expr;
                if (resolve_node_type(ctx, arg, scope) == TYPE_INVALID)
                    return TYPE_INVALID;

                (*astp)->nodes[arglist].info.type_info = TYPE_VOID;
            }

            if (entry->tu_id == ctx->tu_id)
            { /* The function definition exists in our own AST. */
                if ((*astp)->nodes[entry->ast_node].info.node_type
                    == AST_FUNC_TEMPLATE)
                {
                    ast_id func
                        = instantiate_func(ctx, entry->ast_node, ctx, n);
                    if (func < 0)
                        return TYPE_INVALID;
                    entry->ast_node = func;
                }

                /* Can now get the return type by type checking the function
                 * definition */
                ret_type = resolve_node_type(ctx, entry->ast_node, scope);
                if (ret_type == TYPE_INVALID)
                    return TYPE_INVALID;
            }
            else
            { /* The function definition exists in another AST. Since semantic
               * checks are run in parallel, the ASTs are protected by a mutex
               */
                struct mutex* their_mutex = ctx->tu_mutexes[entry->tu_id];
                struct mutex* our_mutex = ctx->tu_mutexes[ctx->tu_id];
                struct ctx their_ctx = create_context_for_tu(ctx, entry->tu_id);

                mutex_unlock(our_mutex);
                mutex_lock(their_mutex);

                if ((*astp)->nodes[entry->ast_node].info.node_type
                    == AST_FUNC_TEMPLATE)
                {
                    ast_id func = instantiate_func(
                        &their_ctx,
                        entry->ast_node,
                        ctx,
                        (*astp)->nodes[n].func_or_container_ref.arglist);
                    if (func < 0)
                    {
                        mutex_unlock(their_mutex);
                        mutex_lock(our_mutex);
                        return TYPE_INVALID;
                    }
                }

                /* Can now get the return type by type checking the function
                 * definition */
                // XXX: How do we get the scope of another AST?
                ret_type = resolve_node_type(&their_ctx, entry->ast_node, 0);
                if (ret_type == TYPE_INVALID)
                {
                    mutex_unlock(their_mutex);
                    mutex_lock(our_mutex);
                    return TYPE_INVALID;
                }
            }

            (*astp)->nodes[identifier].info.type_info = ret_type;
            return (*astp)->nodes[n].info.type_info = ret_type;
        }

        case AST_FUNC_CALL:
            /* This value is already set by AST_FUNC_OR_CONTAINER_REF -- nothing
             * to do here */
            ODBUTIL_DEBUG_ASSERT(0, (void)0);
            return TYPE_INVALID;

        case AST_BOOLEAN_LITERAL:
            return (*astp)->nodes[n].boolean_literal.info.type_info = TYPE_BOOL;
        case AST_BYTE_LITERAL:
            return (*astp)->nodes[n].byte_literal.info.type_info = TYPE_U8;
        case AST_WORD_LITERAL:
            return (*astp)->nodes[n].word_literal.info.type_info = TYPE_U16;
        case AST_INTEGER_LITERAL:
            return (*astp)->nodes[n].integer_literal.info.type_info = TYPE_I32;
        case AST_DWORD_LITERAL:
            return (*astp)->nodes[n].dword_literal.info.type_info = TYPE_U32;
        case AST_DOUBLE_INTEGER_LITERAL:
            return (*astp)->nodes[n].double_integer_literal.info.type_info
                   = TYPE_I64;
        case AST_FLOAT_LITERAL:
            return (*astp)->nodes[n].float_literal.info.type_info = TYPE_F32;
        case AST_DOUBLE_LITERAL:
            return (*astp)->nodes[n].double_literal.info.type_info = TYPE_F64;
        case AST_STRING_LITERAL:
            return (*astp)->nodes[n].string_literal.info.type_info
                   = TYPE_STRING;

        case AST_CAST: {
            ast_id    expr = (*astp)->nodes[n].cast.expr;
            enum type type = resolve_node_type(ctx, expr, scope);
            if (type == TYPE_INVALID)
                return TYPE_INVALID;
            return type_check_cast(*astp, n, filename, source);
        }

        case AST_SCOPE: {
            if (resolve_node_type(ctx, (*astp)->nodes[n].scope.child, scope)
                == TYPE_INVALID)
                return TYPE_INVALID;
            return (*astp)->nodes[n].info.type_info = TYPE_VOID;
        }
    }

not_yet_implemented:
    log_flc_err(
        filename,
        source,
        (*astp)->nodes[n].info.location,
        "Type check not yet implemented for this node.\n");
    log_excerpt_1(source, (*astp)->nodes[n].info.location, "");
    return TYPE_INVALID;
}

#if defined(ODBCOMPILER_AST_SANITY_CHECK)
static int
sanity_check(struct ast* ast, const char* filename, const char* source)
{
    ast_id n;
    int    error = 0;
    int    gutter;
    for (n = 0; n != ast->count; ++n)
    {
        if (ast->nodes[n].info.type_info == TYPE_INVALID)
        {
            ast_id parent;

            /* Function templates are a special case and are allowed to remain
             * in the tree. The reason is because semantic analysis runs in
             * multiple threads, so we have to wait for all checks to complete
             * before we can be sure that the templates are no longer required.
             */
            for (parent = n; parent > -1; parent = ast_find_parent(ast, parent))
                if (ast->nodes[parent].info.node_type == AST_FUNC_TEMPLATE)
                    break;
            if (parent > -1)
                continue;

            log_flc_err(
                filename,
                source,
                ast->nodes[n].info.location,
                "Failed to determine type of AST node id:%d, node_type: %d.\n",
                n,
                ast->nodes[n].info.node_type);
            gutter = log_excerpt_1(source, ast->nodes[n].info.location, "");
            error = -1;
        }

        if (ast->nodes[n].info.node_type == AST_GC)
        {
            log_semantic_err("AST_GC nodes still exist in tree.\n");
            error = -1;
        }
    }

    if (error)
        log_excerpt_note(
            gutter,
            "This should not happen, and means there is a bug in the semantic "
            "analysis of the compiler.\n");

    return error;
}
#endif

static int
type_check(
    struct ast**               tus,
    int                        tu_count,
    int                        tu_id,
    struct mutex**             tu_mutexes,
    const struct utf8*         filenames,
    const struct db_source*    sources,
    const struct plugin_list*  plugins,
    const struct cmd_list*     cmds,
    const struct symbol_table* symbols)
{
    int          return_code;
    struct ast** astp = &tus[tu_id];
    struct ctx   ctx
        = {tus, tu_mutexes, filenames, sources, cmds, symbols, NULL, tu_id};
    typemap_init(&ctx.typemap);

    /*
     * It's necessary to traverse the AST in a way where statements are
     * evaluated in the order they appear in the source code, and in a way
     * where the scope of each block is visited depth-first.
     *
     * This will make it a lot easier to propagate the type information of
     * variables, because they will be processed in the same order the data
     * flows.
     */
    ODBUTIL_DEBUG_ASSERT(
        (*astp)->nodes[(*astp)->root].info.node_type == AST_BLOCK,
        log_semantic_err(
            "type: %d\n", (*astp)->nodes[(*astp)->root].info.node_type));
    mutex_lock(tu_mutexes[tu_id]);
    return_code
        = resolve_node_type(&ctx, (*astp)->root, 0) == TYPE_INVALID ? -1 : 0;
    mutex_unlock(tu_mutexes[tu_id]);

    ast_gc(*astp);
    typemap_deinit(ctx.typemap);

#if defined(ODBCOMPILER_AST_SANITY_CHECK)
    if (return_code == 0)
        if (sanity_check(
                *astp, utf8_cstr(filenames[tu_id]), sources[tu_id].text.data)
            != 0)
        {
            return -1;
        }
#endif

    return return_code;
}

static const struct semantic_check* depends[]
    = {&semantic_loop_for, &semantic_loop_cont, NULL};
const struct semantic_check semantic_type_check = {type_check, depends};
