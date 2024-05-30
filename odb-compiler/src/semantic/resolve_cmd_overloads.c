#include "odb-compiler/ast/ast.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/sdk/type.h"
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
        ast_id    expr = ctx->ast->nodes[arglist].arglist.expr;
        enum type param = vec_get(*params, i)->type;
        enum type arg = ctx->ast->nodes[expr].info.type_info;

        /* Incompatible types */
        switch (type_promote(arg, param))
        {
            case TP_DISALLOW: return 0;

            case TP_NARROWING:
            case TP_STRANGE:
            case TP_ALLOW: continue;
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
        ast_id    expr = ctx->ast->nodes[arglist].arglist.expr;
        enum type param = vec_get(*params, i)->type;
        enum type arg = ctx->ast->nodes[expr].info.type_info;

        switch (type_promote(arg, param))
        {
            case TP_DISALLOW:
            case TP_NARROWING:
            case TP_STRANGE: return 0;

            case TP_ALLOW: continue;
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
        ast_id    expr = ctx->ast->nodes[arglist].arglist.expr;
        enum type param = vec_get(*params, i)->type;
        enum type arg = ctx->ast->nodes[expr].info.type_info;

        if (arg != param)
            return 0;
    }

    return 1;
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
            "{e:error:} ",
            source_filename,
            source.text.data,
            params_loc,
            "Command has ambiguous overloads.\n");
        log_excerpt(source_filename, source.text.data, params_loc, "");
        log_note("", "Available candidates:\n");
        vec_for_each(candidates, pcmd)
        {
            int              i;
            struct utf8_view name = utf8_list_view(&cmds->db_cmd_names, *pcmd);
            const struct param_types_list* param_types
                = vec_get(cmds->param_types, *pcmd);
            const struct utf8_list* param_names
                = vec_get(cmds->db_param_names, *pcmd);
            enum type           ret_type = *vec_get(cmds->return_types, *pcmd);
            plugin_id           plugin_id = *vec_get(cmds->plugin_ids, cmd);
            struct plugin_info* plugin = vec_get(*plugins, plugin_id);
            log_raw(
                "  {emph:%.*s}%s",
                name.len,
                name.data + name.off,
                ret_type == TYPE_VOID ? " " : "(");
            for (i = 0; i != utf8_list_count(param_names); ++i)
            {
                if (i)
                    log_raw(", ");
                log_raw(
                    "%s {u:AS %s}",
                    utf8_list_cstr(param_names, i),
                    type_to_db_name(vec_get(*param_types, i)->type));
            }
            log_raw("%s  ", ret_type == TYPE_VOID ? "" : ")");
            log_raw("[%s]\n", utf8_cstr(plugin->name));
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
            "{e:error:} ",
            source_filename,
            source.text.data,
            params_loc,
            "Parameter mismatch: No version of this command takes the "
            "argument types used here.\n");
        log_excerpt(source_filename, source.text.data, params_loc, "");
        log_note("", "Available candidates:\n");
        cmd_name = utf8_list_view(&cmds->db_cmd_names, cmd);
        for (;
             cmd < cmd_list_count(cmds)
             && utf8_equal(cmd_name, utf8_list_view(&cmds->db_cmd_names, cmd));
             ++cmd)
        {
            int                 i;
            enum type           ret_type = *vec_get(cmds->return_types, cmd);
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
                ret_type == TYPE_VOID ? " " : "(");
            for (i = 0; i != utf8_list_count(param_names); ++i)
            {
                if (i)
                    log_raw(", ");
                log_raw(
                    "%s {u:AS %s}",
                    utf8_list_cstr(param_names, i),
                    type_to_db_name(vec_get(*param_types, i)->type));
            }
            log_raw("%s  ", ret_type == TYPE_VOID ? "" : ")");
            log_raw("[%s]\n", utf8_cstr(plugin->name));
        }
    }
}

static void
log_cmd_signature(
    cmd_id                    cmd_id,
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds)
{
    int              i;
    struct utf8_view name = utf8_list_view(&cmds->db_cmd_names, cmd_id);
    const struct param_types_list* param_types
        = vec_get(cmds->param_types, cmd_id);
    const struct utf8_list* param_names = vec_get(cmds->db_param_names, cmd_id);
    enum type               ret_type = *vec_get(cmds->return_types, cmd_id);
    plugin_id               plugin_id = *vec_get(cmds->plugin_ids, cmd_id);
    const struct plugin_info* plugin = vec_get(*plugins, plugin_id);

    log_note(
        "",
        "{emph:%.*s}%s",
        name.len,
        name.data + name.off,
        ret_type == TYPE_VOID ? " " : "(");
    for (i = 0; i != utf8_list_count(param_names); ++i)
    {
        if (i)
            log_raw(", ");
        log_raw(
            "%s {rhs:AS %s}",
            utf8_list_cstr(param_names, i),
            type_to_db_name(vec_get(*param_types, i)->type));
    }
    log_raw("%s  ", ret_type == TYPE_VOID ? "" : ")");
    log_raw("[%s]\n", utf8_cstr(plugin->name));
}

static void
typecheck_arguments(
    struct ast*               ast,
    ast_id                    cmd_node,
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    const char*               source_filename,
    struct db_source          source)
{
    int                            i;
    cmd_id                         cmd_id = ast->nodes[cmd_node].cmd.id;
    ast_id                         arglist = ast->nodes[cmd_node].cmd.arglist;
    const struct param_types_list* params = vec_get(cmds->param_types, cmd_id);

    ODBSDK_DEBUG_ASSERT(ast->nodes[cmd_node].info.node_type == AST_COMMAND);
    ODBSDK_DEBUG_ASSERT(ast->nodes[arglist].info.node_type == AST_ARGLIST);

    for (i = 0; i != vec_count(*params);
         ++i, arglist = ast->nodes[arglist].arglist.next)
    {
        ast_id    arg = ast->nodes[arglist].arglist.expr;
        enum type arg_type = ast->nodes[arg].info.type_info;
        enum type param_type = vec_get(*params, i)->type;

        switch (type_promote(arg_type, param_type))
        {
            case TP_DISALLOW: ODBSDK_DEBUG_ASSERT(0); break;
            case TP_ALLOW: break;

            case TP_NARROWING:
                log_flc(
                    "{w:warning:} ",
                    source_filename,
                    source.text.data,
                    ast->nodes[arg].info.location,
                    "Narrowing conversion argument %d from {lhs:%s} to "
                    "{rhs:%s} in command call\n",
                    i + 1,
                    type_to_db_name(arg_type),
                    type_to_db_name(param_type));
                log_excerpt(
                    source_filename,
                    source.text.data,
                    ast->nodes[arg].info.location,
                    type_to_db_name(arg_type));
                log_cmd_signature(cmd_id, plugins, cmds);
                break;

            case TP_STRANGE:
                log_flc(
                    "{w:warning:} ",
                    source_filename,
                    source.text.data,
                    ast->nodes[arg].info.location,
                    "Strange conversion of argument %d from {lhs:%s} to "
                    "{rhs:%s} in command call\n",
                    i + 1,
                    type_to_db_name(arg_type),
                    type_to_db_name(param_type));
                log_excerpt(
                    source_filename,
                    source.text.data,
                    ast->nodes[arg].info.location,
                    type_to_db_name(arg_type));
                log_cmd_signature(cmd_id, plugins, cmds);
                break;
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
        if (ast->nodes[n].info.node_type != AST_COMMAND)
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

        /* Filter list of candidates with increasingly strict rules */
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

        typecheck_arguments(ast, n, plugins, cmds, source_filename, source);
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
