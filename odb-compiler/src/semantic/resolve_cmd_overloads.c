#include "odb-compiler/ast/ast.h"
#include "odb-compiler/parser/db_source.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-compiler/semantic/type.h"
#include "odb-util/log.h"
#include "odb-util/vec.h"
#include <assert.h>

VEC_DECLARE_API(static, candidates, cmd_id, 8)
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
    int                                i;
    ast_id                             arglist;
    struct ctx*                        ctx = user;
    const struct cmd_param_types_list* params
        = ctx->cmds->param_types->data[*cmd_id];

    /* param count mismatch */
    if (cmd_param_types_list_count(params) != ctx->argcount)
        return 0;

    for (i = 0, arglist = ctx->arglist; i != ctx->argcount;
         ++i, arglist = ctx->ast->nodes[arglist].arglist.next)
    {
        ast_id    expr = ctx->ast->nodes[arglist].arglist.expr;
        enum type param = params->data[i].type;
        enum type arg = ast_type_info(ctx->ast, expr);

        /* Incompatible types */
        switch (type_convert(arg, param))
        {
            case TC_DISALLOW: return 0;

            case TC_TRUNCATE:
            case TC_SIGN_CHANGE:
            case TC_TRUENESS:
            case TC_INT_TO_FLOAT:
            case TC_BOOL_PROMOTION:
            case TC_ALLOW: continue;
        }
    }

    return 1;
}

static int
eliminate_problematic_casts(cmd_id* cmd_id, void* user)
{
    int                                i;
    ast_id                             arglist;
    struct ctx*                        ctx = user;
    const struct cmd_param_types_list* params
        = ctx->cmds->param_types->data[*cmd_id];

    for (i = 0, arglist = ctx->arglist; i != ctx->argcount;
         ++i, arglist = ctx->ast->nodes[arglist].arglist.next)
    {
        ast_id    expr = ctx->ast->nodes[arglist].arglist.expr;
        enum type param = params->data[i].type;
        enum type arg = ast_type_info(ctx->ast, expr);

        switch (type_convert(arg, param))
        {
            case TC_DISALLOW:
            case TC_TRUNCATE:
            case TC_SIGN_CHANGE:
            case TC_TRUENESS:
            case TC_INT_TO_FLOAT:
            case TC_BOOL_PROMOTION: return 0;

            case TC_ALLOW: continue;
        }
    }

    return 1;
}
static int
eliminate_all_but_exact_matches(cmd_id* cmd_id, void* user)
{
    int                                i;
    ast_id                             arglist;
    struct ctx*                        ctx = user;
    const struct cmd_param_types_list* params
        = ctx->cmds->param_types->data[*cmd_id];

    for (i = 0, arglist = ctx->arglist; i != ctx->argcount;
         ++i, arglist = ctx->ast->nodes[arglist].arglist.next)
    {
        ast_id    expr = ctx->ast->nodes[arglist].arglist.expr;
        enum type param = params->data[i].type;
        enum type arg = ast_type_info(ctx->ast, expr);

        if (arg != param)
            return 0;
    }

    return 1;
}

static void
report_no_commands_found(
    const struct ast*         ast,
    ast_id                    arglist,
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    cmd_id                    cmd,
    const char*               filename,
    const char*               source,
    const struct candidates*  candidates)
{
    int              gutter;
    struct utf8_view cmd_name;

    /* We want to highlight the entire argument list, not just the first. Merge
     * locations of first and last */
    struct utf8_span params_loc = ast->nodes[arglist].info.location;
    while (ast->nodes[arglist].arglist.next > -1)
        arglist = ast->nodes[arglist].arglist.next;
    params_loc.len = ast->nodes[arglist].info.location.off - params_loc.off
                     + ast->nodes[arglist].info.location.len;

    log_flc_err(
        filename,
        source,
        params_loc,
        "Parameter mismatch: No version of this command takes the "
        "argument types used here.\n");
    gutter = log_excerpt_1(source, params_loc, "");
    log_excerpt_note(gutter, "Available candidates:\n");
    cmd_name = utf8_list_view(cmds->db_cmd_names, cmd);
    for (; cmd < cmd_list_count(cmds)
           && utf8_equal(cmd_name, utf8_list_view(cmds->db_cmd_names, cmd));
         ++cmd)
    {
        int                       i;
        enum type                 ret_type = cmds->return_types->data[cmd];
        plugin_id                 plugin_id = cmds->plugin_ids->data[cmd];
        const struct plugin_info* plugin = &plugins->data[plugin_id];
        const struct cmd_param_types_list* param_types
            = cmds->param_types->data[cmd];
        struct utf8_list* param_names = cmds->db_param_names->data[cmd];
        log_raw(
            "%*s|   {emph:%.*s}%s",
            gutter,
            "",
            cmd_name.len,
            cmd_name.data + cmd_name.off,
            ret_type == TYPE_VOID ? " " : "(");
        for (i = 0; i != utf8_list_count(param_names); ++i)
        {
            if (i)
                log_raw(", ");
            log_raw(
                "%s {emph1:AS %s}",
                utf8_list_cstr(param_names, i),
                type_to_db_name(param_types->data[i].type));
        }
        log_raw("%s  ", ret_type == TYPE_VOID ? "" : ")");
        log_raw("[%s]\n", utf8_cstr(plugin->name));
    }
}

static void
report_ambiguous_overloads(
    const struct ast*         ast,
    ast_id                    arglist,
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    cmd_id                    cmd,
    const char*               filename,
    const char*               source,
    const struct candidates*  candidates)
{
    const cmd_id* pcmd;
    int           gutter;

    /* We want to highlight the entire argument list, not just the first. Merge
     * locations of first and last */
    struct utf8_span params_loc = ast->nodes[arglist].info.location;
    while (ast->nodes[arglist].arglist.next > -1)
        arglist = ast->nodes[arglist].arglist.next;
    params_loc.len = ast->nodes[arglist].info.location.off - params_loc.off
                     + ast->nodes[arglist].info.location.len;

    log_flc_err(
        filename, source, params_loc, "Command has ambiguous overloads.\n");
    gutter = log_excerpt_1(source, params_loc, "");
    log_excerpt_note(gutter, "Conflicting overloads are:\n");
    vec_for_each(candidates, pcmd)
    {
        int              i;
        struct utf8_view name = utf8_list_view(cmds->db_cmd_names, *pcmd);
        const struct cmd_param_types_list* param_types
            = cmds->param_types->data[*pcmd];
        struct utf8_list* param_names = cmds->db_param_names->data[*pcmd];
        enum type         ret_type = cmds->return_types->data[*pcmd];
        plugin_id         plugin_id = cmds->plugin_ids->data[cmd];
        const struct plugin_info* plugin = &plugins->data[plugin_id];
        log_raw(
            "%*s|   {emph:%.*s}%s",
            gutter,
            "",
            name.len,
            name.data + name.off,
            ret_type == TYPE_VOID ? " " : "(");
        for (i = 0; i != utf8_list_count(param_names); ++i)
        {
            if (i)
                log_raw(", ");
            log_raw(
                "%s {emph1:AS %s}",
                utf8_list_cstr(param_names, i),
                type_to_db_name(param_types->data[i].type));
        }
        log_raw("%s  ", ret_type == TYPE_VOID ? "" : ")");
        log_raw("[%s]\n", utf8_cstr(plugin->name));
    }
    log_excerpt_note(
        gutter,
        "This is usually an issue with conflicting plugins, or poorly designed "
        "plugins. You can try to fix it by casting the arguments to the types "
        "required.\n");
}

static void
log_cmd_signature(
    cmd_id                    cmd_id,
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    int                       gutter)
{
    int              i;
    struct utf8_view name = utf8_list_view(cmds->db_cmd_names, cmd_id);
    const struct cmd_param_types_list* param_types
        = cmds->param_types->data[cmd_id];
    struct utf8_list*         param_names = cmds->db_param_names->data[cmd_id];
    enum type                 ret_type = cmds->return_types->data[cmd_id];
    plugin_id                 plugin_id = cmds->plugin_ids->data[cmd_id];
    const struct plugin_info* plugin = &plugins->data[plugin_id];

    log_excerpt_note(
        gutter,
        "Calling command: {emph:%.*s}%s",
        name.len,
        name.data + name.off,
        ret_type == TYPE_VOID ? " " : "(");
    for (i = 0; i != utf8_list_count(param_names); ++i)
    {
        if (i)
            log_raw(", ");
        log_raw(
            "%s {emph2:AS %s}",
            utf8_list_cstr(param_names, i),
            type_to_db_name(param_types->data[i].type));
    }
    log_raw("%s  ", ret_type == TYPE_VOID ? "" : ")");
    log_raw("[%s]\n", utf8_cstr(plugin->name));
}

static int
typecheck_warnings(
    struct ast**              astp,
    ast_id                    cmd_node,
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    const char*               filename,
    const char*               source)
{
    int         i;
    struct ast* ast = *astp;
    cmd_id      cmd_id = ast->nodes[cmd_node].cmd.id;
    ast_id      arglist = ast->nodes[cmd_node].cmd.arglist;
    const struct cmd_param_types_list* params = cmds->param_types->data[cmd_id];

    ODBUTIL_DEBUG_ASSERT(
        ast_node_type(ast, cmd_node) == AST_COMMAND,
        log_semantic_err("type: %d\n", ast_node_type(ast, cmd_node)));

    for (i = 0; i != cmd_param_types_list_count(params);
         ++i, arglist = ast->nodes[arglist].arglist.next)
    {
        ODBUTIL_DEBUG_ASSERT(
            ast_node_type(ast, arglist) == AST_ARGLIST,
            log_semantic_err("type: %d\n", ast_node_type(ast, arglist)));
        int       gutter;
        ast_id    arg = ast->nodes[arglist].arglist.expr;
        enum type arg_type = ast_type_info(ast, arg);
        enum type param_type = params->data[i].type;

        switch (type_convert(arg_type, param_type))
        {
            case TC_DISALLOW: ODBUTIL_DEBUG_ASSERT(0, (void)0); break;
            case TC_ALLOW: break;

            case TC_TRUNCATE:
                log_flc_warn(
                    filename,
                    source,
                    ast->nodes[arg].info.location,
                    "Argument %d is truncated in conversion from {emph1:%s} to "
                    "{emph2:%s} in command call.\n",
                    i + 1,
                    type_to_db_name(arg_type),
                    type_to_db_name(param_type));
                gutter = log_excerpt_1(
                    source,
                    ast->nodes[arg].info.location,
                    type_to_db_name(arg_type));
                log_cmd_signature(cmd_id, plugins, cmds, gutter);
                break;

            case TC_SIGN_CHANGE:
            case TC_TRUENESS:
            case TC_INT_TO_FLOAT:
            case TC_BOOL_PROMOTION:
                log_flc_warn(
                    filename,
                    source,
                    ast->nodes[arg].info.location,
                    "Implicit conversion of argument %d from {emph1:%s} to "
                    "{emph2:%s} in command call.\n",
                    i + 1,
                    type_to_db_name(arg_type),
                    type_to_db_name(param_type));
                gutter = log_excerpt_1(
                    source,
                    ast->nodes[arg].info.location,
                    type_to_db_name(arg_type));
                log_cmd_signature(cmd_id, plugins, cmds, gutter);
                break;
        }

        /* Insert cast to correct type if necessary */
        if (arg_type != param_type)
        {
            ast_id cast = ast_cast(
                astp, arg, param_type, ast->nodes[arg].info.location);
            if (cast < -1)
                return -1;
            ast = *astp;
            ast->nodes[arglist].arglist.expr = cast;
        }
    }

    return 0;
}

static int
resolve_cmd_overloads(
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
    ast_id             n;
    ast_id             arglist;
    cmd_id             cmd;
    struct utf8_view   cmd_name;
    struct candidates* candidates;
    struct candidates* prev_candidates;

    struct ast** astp = &tus[tu_id];
    struct ast*  ast = *astp;
    const char*  filename = utf8_cstr(filenames[tu_id]);
    const char*  source = sources[tu_id].text.data;
    struct ctx   ctx = {ast, cmds, 0, -1};

    candidates_init(&candidates);
    candidates_init(&prev_candidates);

    for (n = 0; n != ast_count(ast); ++n)
    {
        if (ast_node_type(ast, n) != AST_COMMAND)
            continue;

        /* Build a list of candidates.
         * All overloads will be next to each other in memory, where the ID
         * assigned to the AST node will be the first overload (because it was
         * found using utf8_lower_bound()). */
        candidates_clear(candidates);
        cmd = ast->nodes[n].cmd.id;
        cmd_name = utf8_list_view(cmds->db_cmd_names, cmd);
        do
        {
            if (candidates_push(&candidates, cmd++) != 0)
                goto fail;
        } while (
            cmd < cmd_list_count(cmds)
            && utf8_equal(cmd_name, utf8_list_view(cmds->db_cmd_names, cmd)));

        /* Count number of arguments in the AST */
        ctx.argcount = 0;
        for (arglist = ast->nodes[n].cmd.arglist; arglist > -1;
             arglist = ast->nodes[arglist].arglist.next)
            ctx.argcount++;

        /* Filter list of candidates with increasingly strict rules */
        ctx.arglist = ast->nodes[n].cmd.arglist;
        candidates_retain(
            candidates, eliminate_obviously_wrong_overloads, &ctx);
        if (candidates_count(candidates) > 1)
            candidates_retain(candidates, eliminate_problematic_casts, &ctx);

        /* Have to be as strict as possible. This might eliminate all commands,
         * in which case we want to report an ambiguous overload error using the
         * candidates list as it is now. Therefore, make a copy */
        if (candidates_resize(&prev_candidates, candidates_count(candidates))
            < 0)
            goto fail;
        memcpy(
            prev_candidates->data,
            candidates->data,
            sizeof(*candidates->data) * candidates_count(candidates));

        if (candidates_count(candidates) > 1)
            candidates_retain(
                candidates, eliminate_all_but_exact_matches, &ctx);

        /* Success: Update command ID in AST and perform any necessary type
         * conversions */
        if (candidates_count(candidates) == 1)
        {
            ast->nodes[n].cmd.id = *vec_first(candidates);
            if (typecheck_warnings(astp, n, plugins, cmds, filename, source)
                != 0)
            {
                goto fail;
            }

            ast = *astp;
            continue;
        }

        if (candidates_count(candidates) == 0)
            report_no_commands_found(
                ast,
                ast->nodes[n].cmd.arglist,
                plugins,
                cmds,
                ast->nodes[n].cmd.id,
                filename,
                source,
                candidates);
        else
            report_ambiguous_overloads(
                ast,
                ast->nodes[n].cmd.arglist,
                plugins,
                cmds,
                ast->nodes[n].cmd.id,
                filename,
                source,
                prev_candidates);

        goto fail;
    }

    candidates_deinit(prev_candidates);
    candidates_deinit(candidates);
    return 0;

fail:
    candidates_deinit(prev_candidates);
    candidates_deinit(candidates);
    return -1;
}

static const struct semantic_check* depends[] = {&semantic_type_check, NULL};

const struct semantic_check semantic_resolve_cmd_overloads
    = {resolve_cmd_overloads, depends, "resolve_cmd_overloads"};
