#include "odb-compiler/ast/ast.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-sdk/config.h"
#include "odb-sdk/log.h"
#include "odb-sdk/vec.h"
#include <assert.h>

VEC_DECLARE_API(candidates, cmd_id, 8)
VEC_DEFINE_API(candidates, cmd_id, 8)

struct ctx
{
    struct ast*            ast;
    const struct cmd_list* cmds;
    ast_id                 argcount;
    ast_id                 arglist;
};

static enum cmd_param_type
expr_to_type(const struct ast* ast, ast_id expr, const struct cmd_list* cmds)
{
    ODBSDK_DEBUG_ASSERT(expr > -1);
    switch (ast->nodes[expr].info.type)
    {
        case AST_BLOCK:
        case AST_ARGLIST:
        case AST_CONST_DECL:
        case AST_ASSIGN: return CMD_PARAM_VOID;

        case AST_COMMAND:
            return *vec_get(cmds->return_types, ast->nodes[expr].cmd.id);

        case AST_IDENTIFIER: break;

        case AST_BOOLEAN_LITERAL: return CMD_PARAM_BOOLEAN;
        case AST_BYTE_LITERAL: return CMD_PARAM_BYTE;
        case AST_WORD_LITERAL: return CMD_PARAM_WORD;
        case AST_DWORD_LITERAL: return CMD_PARAM_DWORD;
        case AST_INTEGER_LITERAL: return CMD_PARAM_INTEGER;
        case AST_DOUBLE_INTEGER_LITERAL: return CMD_PARAM_LONG;
        case AST_FLOAT_LITERAL: return CMD_PARAM_FLOAT;
        case AST_DOUBLE_LITERAL: return CMD_PARAM_DOUBLE;
        case AST_STRING_LITERAL: return CMD_PARAM_STRING;
    }

    log_err(
        "[sem] ",
        "Deducing type of AST node type %d is not implemented\n",
        ast->nodes[expr].info.type);
    return CMD_PARAM_VOID;
}

static int
type_to_idx(enum cmd_param_type type)
{
    switch (type)
    {
        case CMD_PARAM_VOID: return 0;
        case CMD_PARAM_LONG: return 1;
        case CMD_PARAM_DWORD: return 2;
        case CMD_PARAM_INTEGER: return 3;
        case CMD_PARAM_WORD: return 4;
        case CMD_PARAM_BYTE: return 5;
        case CMD_PARAM_BOOLEAN: return 6;
        case CMD_PARAM_FLOAT: return 7;
        case CMD_PARAM_DOUBLE: return 8;
        case CMD_PARAM_STRING: return 9;
        case CMD_PARAM_ARRAY: return 10;
        case CMD_PARAM_LABEL: return 11;
        case CMD_PARAM_DABEL: return 12;
        case CMD_PARAM_ANY: return 13;
        case CMD_PARAM_USER_DEFINED_VAR_PTR: return 14;
    }

    log_warn("[sem] ", "Unknown type %d passed to type_to_idx()\n", type);
    return 0;
}

enum type_promotion_result
{
    DISALLOW = 0,
    ALLOW = 1,
    LOSS_OF_INFO = 2,
    STRANGE = 3
};
static enum type_promotion_result
type_can_be_promoted_to(enum cmd_param_type from, enum cmd_param_type to)
{
    /* clang-format off */
    static enum type_promotion_result rules[15][15] = {
/*       TO */
/*FROM   0 R D L W Y B F O S H P Q X E */
/* 0 */ {1,0,0,0,0,0,0,0,0,0,0,0,0,3,0}, /* VOID */
/* R */ {0,1,2,2,2,2,3,2,2,0,0,0,0,3,0}, /* LONG */
/* D */ {0,1,1,2,2,2,3,2,2,0,0,0,0,3,0}, /* DWORD */
/* L */ {0,1,2,1,2,2,3,2,2,0,0,0,0,3,0}, /* INTEGER */
/* W */ {0,1,1,1,1,2,3,2,2,0,0,0,0,3,0}, /* WORD */
/* Y */ {0,1,1,1,1,1,3,2,2,0,0,0,0,3,0}, /* BYTE */
/* B */ {0,3,3,3,3,3,1,3,3,0,0,0,0,3,0}, /* BOOLEAN */
/* F */ {0,2,2,2,2,2,3,1,1,0,0,0,0,3,0}, /* FLOAT */
/* O */ {0,2,2,2,2,2,3,2,1,0,0,0,0,3,0}, /* DOUBLE */
/* S */ {0,0,0,0,0,0,0,0,0,1,0,0,0,3,0}, /* STRING */
/* H */ {0,0,0,0,0,0,0,0,0,0,1,0,0,3,0}, /* ARRAY */
/* P */ {0,0,0,0,0,0,0,0,0,0,0,1,0,3,0}, /* LABEL */
/* Q */ {0,0,0,0,0,0,0,0,0,0,0,0,1,3,0}, /* DLABEL */
/* X */ {3,3,3,3,3,3,3,3,3,3,3,3,3,1,0}, /* ANY (reinterpret)*/
/* E */ {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, /* USER DEFINED */
    };
    /* clang-format on */
    return rules[type_to_idx(from)][type_to_idx(to)];
}

static int
eliminate_obviously_wrong_overloads(cmd_id* cmd_id, void* user)
{
    int                      i;
    ast_id                   arglist;
    struct ctx*              ctx = user;
    struct param_types_list* params = vec_get(ctx->cmds->param_types, *cmd_id);

    /* param count mismatch */
    if (vec_count(*params) != ctx->argcount)
        return 0;

    for (i = 0, arglist = ctx->arglist; i != ctx->argcount;
         ++i, arglist = ctx->ast->nodes[arglist].arglist.next)
    {
        ast_id              expr = ctx->ast->nodes[arglist].arglist.expr;
        enum cmd_param_type param = vec_get(*params, i)->type;
        enum cmd_param_type arg = expr_to_type(ctx->ast, expr, ctx->cmds);

        /* Incompatible types */
        switch (type_can_be_promoted_to(arg, param))
        {
            case DISALLOW: return 0;

            case LOSS_OF_INFO:
            case STRANGE:
            case ALLOW: continue;
        }
    }

    return 1;
}

static int
eliminate_problematic_casts(cmd_id* cmd_id, void* user)
{
    int                      i;
    ast_id                   arglist;
    struct ctx*              ctx = user;
    struct param_types_list* params = vec_get(ctx->cmds->param_types, *cmd_id);

    for (i = 0, arglist = ctx->arglist; i != ctx->argcount;
         ++i, arglist = ctx->ast->nodes[arglist].arglist.next)
    {
        ast_id              expr = ctx->ast->nodes[arglist].arglist.expr;
        enum cmd_param_type param = vec_get(*params, i)->type;
        enum cmd_param_type arg = expr_to_type(ctx->ast, expr, ctx->cmds);

        switch (type_can_be_promoted_to(arg, param))
        {
            case DISALLOW:
            case LOSS_OF_INFO:
            case STRANGE: return 0;

            case ALLOW: continue;
        }
    }

    return 1;
}
static int
eliminate_all_but_exact_matches(cmd_id* cmd_id, void* user)
{
    int                      i;
    ast_id                   arglist;
    struct ctx*              ctx = user;
    struct param_types_list* params = vec_get(ctx->cmds->param_types, *cmd_id);

    for (i = 0, arglist = ctx->arglist; i != ctx->argcount;
         ++i, arglist = ctx->ast->nodes[arglist].arglist.next)
    {
        ast_id              expr = ctx->ast->nodes[arglist].arglist.expr;
        enum cmd_param_type param = vec_get(*params, i)->type;
        enum cmd_param_type arg = expr_to_type(ctx->ast, expr, ctx->cmds);

        if (arg != param)
            return 0;
    }

    return 1;
}

static const char*
type_as_name(enum cmd_param_type type)
{
    switch (type)
    {
        case CMD_PARAM_VOID: break;
        case CMD_PARAM_LONG: return "AS DOUBLE INTEGER";
        case CMD_PARAM_DWORD: return "AS DWORD";
        case CMD_PARAM_INTEGER: return "AS INTEGER";
        case CMD_PARAM_WORD: return "AS WORD";
        case CMD_PARAM_BYTE: return "AS BYTE";
        case CMD_PARAM_BOOLEAN: return "AS BOOLEAN";
        case CMD_PARAM_FLOAT: return "AS FLOAT";
        case CMD_PARAM_DOUBLE: return "AS DOUBLE";
        case CMD_PARAM_STRING: return "AS STRING";
        case CMD_PARAM_ARRAY: break;
        case CMD_PARAM_LABEL: break;
        case CMD_PARAM_DABEL: break;
        case CMD_PARAM_ANY: return "AS ANY"; break;
        case CMD_PARAM_USER_DEFINED_VAR_PTR: return "...";
    }

    return "";
}

static void
report_error(
    const struct ast*         ast,
    ast_id                    arglist,
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    cmd_id                    cmd,
    const char*               source_filename,
    struct db_source          source,
    struct candidates         candidates)
{
    /* We want to highlight the entire argument list, not just the first. Merge
     * locations of first and last */
    struct utf8_span params_loc = ast->nodes[arglist].info.location;
    while (ast->nodes[arglist].arglist.next > -1)
        arglist = ast->nodes[arglist].arglist.next;
    params_loc.len = ast->nodes[arglist].info.location.off - params_loc.off
                     + ast->nodes[arglist].info.location.len;

    if (vec_count(candidates) > 1)
    {
        cmd_id* pcmd;
        log_flc(
            "{e:semantic error:} ",
            source_filename,
            source.text.data,
            params_loc,
            "Parameter mismatch: Command has ambiguous overloads.\n");
        log_excerpt(source_filename, source.text.data, params_loc);
        log_flc(
            "{n:note:} ",
            source_filename,
            source.text.data,
            params_loc,
            "Available candidates:\n");
        vec_for_each(candidates, pcmd)
        {
            int              i;
            struct utf8_view name = utf8_list_view(&cmds->db_cmd_names, *pcmd);
            const struct param_types_list* param_types
                = vec_get(cmds->param_types, *pcmd);
            const struct utf8_list* param_names
                = vec_get(cmds->db_param_names, *pcmd);
            enum cmd_param_type ret_type = *vec_get(cmds->return_types, *pcmd);
            plugin_id           plugin_id = *vec_get(cmds->plugin_ids, cmd);
            struct plugin_info* plugin = vec_get(*plugins, plugin_id);
            log_raw(
                "  {emph:%.*s}%s",
                name.len,
                name.data + name.off,
                ret_type == CMD_PARAM_VOID ? " " : "(");
            for (i = 0; i != utf8_list_count(param_names); ++i)
            {
                if (i)
                    log_raw(", ");
                log_raw(
                    "%s {u:%s}",
                    utf8_list_cstr(param_names, i),
                    type_as_name(vec_get(*param_types, i)->type));
            }
            log_raw("%s\n", ret_type == CMD_PARAM_VOID ? "" : ")");
            log_raw("(%s)\n", utf8_cstr(plugin->name));
        }
        log_flc(
            "{n:note:} ",
            source_filename,
            source.text.data,
            params_loc,
            "This is usually an issue with conflicting plugins, or poorly "
            "designed plugins. You can try to fix it by passing more "
            "explicit parameter types to the command.\n");
    }

    if (vec_count(candidates) == 0)
    {
        struct utf8_view cmd_name;
        log_flc(
            "{e:semantic error:} ",
            source_filename,
            source.text.data,
            params_loc,
            "Parameter mismatch: No version of this command takes the "
            "argument types used here.\n");
        log_excerpt(source_filename, source.text.data, params_loc);

        log_flc(
            "{n:note:} ",
            source_filename,
            source.text.data,
            params_loc,
            "Available candidates:\n");
        cmd_name = utf8_list_view(&cmds->db_cmd_names, cmd);
        for (;
             cmd < cmd_list_count(cmds)
             && utf8_equal(cmd_name, utf8_list_view(&cmds->db_cmd_names, cmd));
             ++cmd)
        {
            int                 i;
            enum cmd_param_type ret_type = *vec_get(cmds->return_types, cmd);
            plugin_id           plugin_id = *vec_get(cmds->plugin_ids, cmd);
            struct plugin_info* plugin = vec_get(*plugins, plugin_id);
            const struct param_types_list* param_types
                = vec_get(cmds->param_types, cmd);
            const struct utf8_list* param_names
                = vec_get(cmds->db_param_names, cmd);
            log_raw(
                "  {emph:%.*s}%s",
                cmd_name.len,
                cmd_name.data + cmd_name.off,
                ret_type == CMD_PARAM_VOID ? " " : "(");
            for (i = 0; i != utf8_list_count(param_names); ++i)
            {
                if (i)
                    log_raw(", ");
                log_raw(
                    "%s {u:%s}",
                    utf8_list_cstr(param_names, i),
                    type_as_name(vec_get(*param_types, i)->type));
            }
            log_raw("%s  ", ret_type == CMD_PARAM_VOID ? "" : ")");
            log_raw("(%s)\n", utf8_cstr(plugin->name));
        }
    }
}

static int
resolve_cmd_overloads(
    struct ast*               ast,
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    const char*               source_filename,
    struct db_source          source)
{
    struct utf8_view  cmd_name;
    struct candidates candidates;
    ast_id            n;
    ast_id            paramlist;
    cmd_id            cmd;

    struct ctx ctx = {ast, cmds, 0, -1};

    candidates_init(&candidates);

    for (n = 0; n != ast->node_count; ++n)
    {
        if (ast->nodes[n].info.type != AST_COMMAND)
            continue;

        /* Build a list of candidates.
         * All overloads will be next to each other in memory, where the ID
         * assigned to the AST node will be the first overload (because it was
         * found using utf8_lower_bound()). */
        candidates_clear(candidates);
        cmd = ast->nodes[n].cmd.id;
        cmd_name = utf8_list_view(&cmds->db_cmd_names, cmd);
        do
        {
            if (candidates_push(&candidates, cmd++) != 0)
                goto fail;
        } while (
            cmd < cmd_list_count(cmds)
            && utf8_equal(cmd_name, utf8_list_view(&cmds->db_cmd_names, cmd)));

        /* Count number of arguments in the AST */
        ctx.argcount = 0;
        for (paramlist = ast->nodes[n].cmd.arglist; paramlist > -1;
             paramlist = ast->nodes[paramlist].arglist.next)
            ctx.argcount++;

        /* Set up context */
        ctx.arglist = ast->nodes[n].cmd.arglist;

        candidates_retain(
            candidates, eliminate_obviously_wrong_overloads, &ctx);

        if (vec_count(candidates) > 1)
            candidates_retain(
                candidates, eliminate_all_but_exact_matches, &ctx);

        if (vec_count(candidates) > 1)
            candidates_retain(candidates, eliminate_problematic_casts, &ctx);

        if (vec_count(candidates) != 1)
        {
            report_error(
                ast,
                ast->nodes[n].cmd.arglist,
                plugins,
                cmds,
                ast->nodes[n].cmd.id,
                source_filename,
                source,
                candidates);
            goto fail;
        }

        /* Update command ID in AST */
        ast->nodes[n].cmd.id = *vec_first(candidates);
    }

    candidates_deinit(&candidates);
    return 0;

fail:
    candidates_deinit(&candidates);
    return -1;
}

static const struct semantic_check* depends[]
    = {&semantic_expand_constant_declarations,
       &semantic_type_check_expressions,
       NULL};

const struct semantic_check semantic_resolve_cmd_overloads
    = {depends, resolve_cmd_overloads};
