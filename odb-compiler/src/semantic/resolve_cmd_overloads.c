#include "odb-compiler/ast/ast.h"
#include "odb-compiler/parser/db_source.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-compiler/semantic/type.h"
#include "odb-util/log.h"
#include "odb-util/vec.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>

VEC_DECLARE_API(static, candidates, cmd_id, 8)
VEC_DEFINE_API(candidates, cmd_id, 8)

typedef int (*conversion_valid_func)(enum type, enum type);

struct ctx
{
    conversion_valid_func  is_conversion_valid;
    struct ast*            ast;
    const struct cmd_list* cmds;
    ast_id                 argcount;
    ast_id                 arglist;
};

static int
allow_all(enum type from, enum type to)
{
    switch (type_convert(from, to))
    {
        case TC_DISALLOW: break;

        case TC_TRUNCATE:
        case TC_SIGN_CHANGE:
        case TC_TRUENESS:
        case TC_INT_TO_FLOAT:
        case TC_BOOL_PROMOTION:
        case TC_ALLOW: return 1;
    }
    return 0;
}

static int
allow_non_information_losing_casts(enum type from, enum type to)
{
    switch (type_convert(from, to))
    {
        case TC_DISALLOW:
        case TC_TRUNCATE:
        case TC_INT_TO_FLOAT: break;

        case TC_SIGN_CHANGE:
        case TC_TRUENESS:
        case TC_BOOL_PROMOTION:
        case TC_ALLOW: return 1;
    }
    return 0;
}

static int
allow_non_problematic_casts(enum type from, enum type to)
{
    switch (type_convert(from, to))
    {
        case TC_DISALLOW:
        case TC_TRUNCATE:
        case TC_SIGN_CHANGE:
        case TC_TRUENESS:
        case TC_INT_TO_FLOAT:
        case TC_BOOL_PROMOTION: break;

        case TC_ALLOW: return 1;
    }
    return 0;
}

static int
exact_match(enum type from, enum type to)
{
    return from == to;
}

static const conversion_valid_func narrowing_rules[]
    = {allow_all,
       allow_non_information_losing_casts,
       allow_non_problematic_casts,
       exact_match,
       NULL};

static int
eliminate_candidates(cmd_id* cmd_id, void* user)
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

        if (!ctx->is_conversion_valid(arg, param))
            return 0;
    }

    return 1;
}

static int
arg_is_highlighted(const int* arg_positions, int arg_idx, int hl_count)
{
    int i;
    for (i = 0; i != hl_count; ++i)
        if (arg_positions[i] == arg_idx + 1)
            return 1;
    return 0;
}

static void
report_duplicate_commands(
    const struct ast*         ast,
    ast_id                    cmd,
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    const char*               filename,
    const char*               source,
    const struct candidates*  candidates)
{
    const cmd_id* cmdp;
    int           gutter, arg_idx;

    log_flc_err(
        filename,
        source,
        ast_loc(ast, cmd),
        "Command has multiple definitions.\n");
    gutter = log_excerpt_1(source, ast_loc(ast, cmd), "", 0);

    log_excerpt_note(gutter, "Conflicting definitions are:\n");
    vec_for_each(candidates, cmdp)
    {
        struct utf8_view name = utf8_list_view(cmds->db_cmd_names, *cmdp);
        const struct cmd_param_types_list* param_types
            = cmds->param_types->data[*cmdp];
        struct utf8_list* param_names = cmds->db_param_names->data[*cmdp];
        enum type         ret_type = cmds->return_types->data[*cmdp];
        plugin_id         plugin_id = cmds->plugin_ids->data[*cmdp];
        const struct plugin_info* plugin = &plugins->data[plugin_id];
        log_raw(
            "%*s|   {emph:%.*s}%s",
            gutter,
            "",
            name.len,
            name.data + name.off,
            ret_type == TYPE_VOID ? " " : "(");
        for (arg_idx = 0; arg_idx != utf8_list_count(param_names); ++arg_idx)
        {
            if (arg_idx)
                log_raw(", ");
            log_raw(
                "%s AS %s",
                utf8_list_cstr(param_names, arg_idx),
                type_to_db_name(param_types->data[arg_idx].type));
        }
        log_raw("%s  ", ret_type == TYPE_VOID ? "" : ")");
        log_raw("[%s]\n", utf8_cstr(plugin->name));
    }

    log_excerpt_help(
        gutter,
        "You need to uninstall one of the conflicting plugins to fix this "
        "issue.\n");
}

static void
report_ambiguous_overloads(
    const struct ast*         ast,
    ast_id                    arglist,
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    const char*               filename,
    const char*               source,
    int                       rule_idx,
    const struct candidates*  candidates)
{
    const cmd_id*         cmdp;
    int                   gutter;
    int                   arg_idx, hl_count;
    ast_id                arg;
    struct log_highlight* hl;
    int*                  arg_positions;

    int param_count = cmd_param_types_list_count(
        cmds->param_types->data[*vec_first(candidates)]);

    ODBUTIL_DEBUG_ASSERT(param_count > 0, (void)0);
    ODBUTIL_DEBUG_ASSERT(narrowing_rules[rule_idx] != NULL, (void)0);

    hl = mem_alloc(sizeof(*hl) * (param_count + 1));
    arg_positions = mem_alloc(sizeof(*arg_positions) * param_count);

    log_flc_err(
        filename,
        source,
        ast_loc(ast, arglist),
        "Command has ambiguous overloads. ");

    hl_count = 0;
    memset(hl, 0, sizeof(*hl) * (param_count + 1));
    for (arg = arglist, arg_idx = 0; arg > -1;
         arg = ast->nodes[arg].arglist.next, ++arg_idx)
    {
        ast_id    expr = ast->nodes[arg].arglist.expr;
        enum type arg_type = ast_type_info(ast, expr);
        vec_for_each(candidates, cmdp)
        {
            enum type param_type
                = cmds->param_types->data[*cmdp]->data[arg_idx].type;

            if (narrowing_rules[rule_idx](arg_type, param_type))
            {
                struct log_highlight item
                    = {"",
                       type_to_db_name(arg_type),
                       ast_loc(ast, expr),
                       LOG_HIGHLIGHT,
                       LOG_MARKERS,
                       hl_count};
                hl[hl_count] = item;
                arg_positions[hl_count] = arg_idx + 1;
                ++hl_count;
                break;
            }
        }
    }

    /* In some cases we fail to determine which arguments exactly didn't match.
     * Highlight every argument in this case */
    if (hl_count == 0)
    {
        for (arg = arglist, arg_idx = 0; arg > -1;
             arg = ast->nodes[arg].arglist.next, ++arg_idx)
        {
            ast_id               expr = ast->nodes[arg].arglist.expr;
            enum type            arg_type = ast_type_info(ast, expr);
            struct log_highlight item
                = {"",
                   type_to_db_name(arg_type),
                   ast_loc(ast, expr),
                   LOG_HIGHLIGHT,
                   LOG_MARKERS,
                   0};
            hl[hl_count] = item;
            arg_positions[hl_count] = arg_idx + 1;
            ++hl_count;
        }
    }

    log_raw("Unable to match argument%s ", hl_count > 1 ? "s" : "");
    for (arg_idx = 0; arg_idx != hl_count; ++arg_idx)
    {
        char fmt[20];
        if (arg_idx > 0 && arg_idx == hl_count - 1)
            log_raw(" and ");
        else if (arg_idx > 0)
            log_raw(", ");
        sprintf(fmt, "{emph%d:%%d}", arg_idx);
        log_raw(fmt, arg_positions[arg_idx]);
    }
    log_raw(" to command signature.\n");

    gutter = log_excerpt(source, hl);
    log_excerpt_note(gutter, "Conflicting overloads are:\n");

    vec_for_each(candidates, cmdp)
    {
        struct utf8_view name = utf8_list_view(cmds->db_cmd_names, *cmdp);
        const struct cmd_param_types_list* param_types
            = cmds->param_types->data[*cmdp];
        struct utf8_list* param_names = cmds->db_param_names->data[*cmdp];
        enum type         ret_type = cmds->return_types->data[*cmdp];
        plugin_id         plugin_id = cmds->plugin_ids->data[*cmdp];
        const struct plugin_info* plugin = &plugins->data[plugin_id];
        log_raw(
            "%*s|   {emph:%.*s}%s",
            gutter,
            "",
            name.len,
            name.data + name.off,
            ret_type == TYPE_VOID ? " " : "(");
        for (arg_idx = 0; arg_idx != utf8_list_count(param_names); ++arg_idx)
        {
            char fmt[26];
            if (arg_idx)
                log_raw(", ");
            if (arg_is_highlighted(arg_positions, arg_idx, hl_count))
                sprintf(fmt, "{emph%d:%%s AS %%s}", arg_idx);
            else
                strcpy(fmt, "%s AS %s");
            log_raw(
                fmt,
                utf8_list_cstr(param_names, arg_idx),
                type_to_db_name(param_types->data[arg_idx].type));
        }
        log_raw("%s  ", ret_type == TYPE_VOID ? "" : ")");
        log_raw("[%s]\n", utf8_cstr(plugin->name));
    }

    log_excerpt_help(
        gutter,
        "This is usually an issue with conflicting plugins, or poorly designed "
        "plugins. You can try to fix it by explicitly casting the argument%s "
        "to the types required:\n",
        hl_count > 1 ? "s" : "");

    hl_count = 0;
    for (arg = arglist, arg_idx = 0; arg > -1;
         arg = ast->nodes[arg].arglist.next, ++arg_idx)
    {
        ast_id               expr = ast->nodes[arg].arglist.expr;
        struct utf8_span     loc = ast_loc(ast, expr);
        utf8_idx             loc_end = loc.off + loc.len;
        struct log_highlight item
            = {" AS <TYPE>", "", {loc_end, 7}, LOG_INSERT, LOG_MARKERS, 0};

        if (arg_positions[hl_count] != arg_idx + 1)
            continue;

        hl[hl_count++] = item;
    }
    log_excerpt(source, hl);

    mem_free(arg_positions);
    mem_free(hl);
}

static void
report_available_commands(
    const char*               msg,
    const struct ast*         ast,
    ast_id                    arglist,
    const struct plugin_list* plugins,
    const struct cmd_list*    cmds,
    cmd_id                    cmd,
    const char*               filename,
    const char*               source)
{
    int              gutter;
    struct utf8_view cmd_name;

    log_flc_err(
        filename,
        source,
        ast->nodes[arglist].arglist.combined_location,
        "%s",
        msg);
    gutter = log_excerpt_1(
        source, ast->nodes[arglist].arglist.combined_location, "", 0);
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
                "%s {emph0:AS %s}",
                utf8_list_cstr(param_names, i),
                type_to_db_name(param_types->data[i].type));
        }
        log_raw("%s  ", ret_type == TYPE_VOID ? "" : ")");
        log_raw("[%s]\n", utf8_cstr(plugin->name));
    }
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
            "%s {emph0:AS %s}",
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
                    ast_loc(ast, arg),
                    "Argument %d is truncated in conversion from {emph0:%s} to "
                    "{emph1:%s} in command call.\n",
                    i + 1,
                    type_to_db_name(arg_type),
                    type_to_db_name(param_type));
                gutter = log_excerpt_1(
                    source, ast_loc(ast, arg), type_to_db_name(arg_type), 0);
                log_cmd_signature(cmd_id, plugins, cmds, gutter);
                break;

            case TC_SIGN_CHANGE:
            case TC_TRUENESS:
            case TC_INT_TO_FLOAT:
            case TC_BOOL_PROMOTION:
                log_flc_warn(
                    filename,
                    source,
                    ast_loc(ast, arg),
                    "Implicit conversion of argument %d from {emph0:%s} to "
                    "{emph1:%s} in command call.\n",
                    i + 1,
                    type_to_db_name(arg_type),
                    type_to_db_name(param_type));
                gutter = log_excerpt_1(
                    source, ast_loc(ast, arg), type_to_db_name(arg_type), 0);
                log_cmd_signature(cmd_id, plugins, cmds, gutter);
                break;
        }

        /* Insert cast to correct type if necessary */
        if (arg_type != param_type)
        {
            ast_id cast = ast_cast(astp, arg, param_type, ast_loc(ast, arg));
            if (cast < -1)
                return -1;
            ast = *astp;
            ast->nodes[arglist].arglist.expr = cast;
            ast->nodes[cast].info.type_info = param_type;
        }
    }

    return 0;
}

static int
create_candidates_list(
    struct candidates** candidates, const struct cmd_list* cmds, cmd_id cmd_id)
{
    /* All overloads will be next to each other in memory, where the ID
     * assigned to the AST node will be the first overload (because it was
     * found using utf8_lower_bound()). */
    struct utf8_view cmd_name = utf8_list_view(cmds->db_cmd_names, cmd_id);
    do
    {
        if (candidates_push(candidates, cmd_id++) != 0)
            return -1;
    } while (
        cmd_id < cmd_list_count(cmds)
        && utf8_equal(cmd_name, utf8_list_view(cmds->db_cmd_names, cmd_id)));

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
    int                rule_idx, param_min, param_max;
    struct candidates* candidates;
    struct candidates* prev_candidates;
    cmd_id*            cmdp;

    struct ast** astp = &tus[tu_id];
    struct ast*  ast = *astp;
    const char*  filename = utf8_cstr(filenames[tu_id]);
    const char*  source = sources[tu_id].text.data;
    struct ctx   ctx = {NULL, ast, cmds, 0, -1};

    candidates_init(&candidates);
    candidates_init(&prev_candidates);

    for (n = 0; n != ast_count(ast); ++n)
    {
        if (ast_node_type(ast, n) != AST_COMMAND)
            continue;

        /* Skip processing commands that exist in polymorphic functions */
        if (ast_type_info(ast, n) == TYPE_INVALID)
            continue;

        /* Collect all overloads of the command */
        candidates_clear(candidates);
        if (create_candidates_list(&candidates, cmds, ast->nodes[n].cmd.id)
            != 0)
        {
            goto fail;
        }

        /* Count number of arguments in the AST */
        ctx.arglist = ast->nodes[n].cmd.arglist;
        ctx.argcount = 0;
        for (arglist = ast->nodes[n].cmd.arglist; arglist > -1;
             arglist = ast->nodes[arglist].arglist.next)
            ctx.argcount++;

        /* Determine min and max number of parameters of all overloads. This is
         * used later for error reporting */
        param_min = INT_MAX;
        param_max = 0;
        vec_for_each(candidates, cmdp)
        {
            const struct cmd_param_types_list* params
                = cmds->param_types->data[*cmdp];
            int param_count = cmd_param_types_list_count(params);
            if (param_count < param_min)
                param_min = param_count;
            if (param_count > param_max)
                param_max = param_count;
        }

        /* The first rule only eliminates impossible conversions. The loop is
         * written so that if we eliminate all candidates in the first rule,
         * then prev_candidates will also be empty. */
        candidates_clear(prev_candidates);
        for (rule_idx = 0; narrowing_rules[rule_idx] != NULL; ++rule_idx)
        {
            /* Filter list of candidates with increasingly strict rules */
            ctx.is_conversion_valid = narrowing_rules[rule_idx];
            candidates_retain(candidates, eliminate_candidates, &ctx);

            if (candidates_count(candidates) < 2)
                break;

            /* Each elimination process might eliminate all remaining commands,
             * in which case we want to report an ambiguous overload error using
             * the candidates list from the previous iteration. Therefore, make
             * a copy */
            if (candidates_resize(
                    &prev_candidates, candidates_count(candidates))
                < 0)
            {
                goto fail;
            }

            memcpy(
                prev_candidates->data,
                candidates->data,
                sizeof(*candidates->data) * candidates_count(candidates));
        }

        /* Success: Update command ID in AST and perform any necessary type
         * conversions */
        if (candidates_count(candidates) == 1
            || candidates_count(prev_candidates) == 1)
        {
            ast->nodes[n].cmd.id = candidates_count(candidates) == 1
                                       ? *vec_first(candidates)
                                       : *vec_first(prev_candidates);
            if (typecheck_warnings(astp, n, plugins, cmds, filename, source)
                != 0)
            {
                goto fail;
            }

            ast = *astp;
            continue;
        }

        if (candidates_count(candidates) > 0)
        {
            report_duplicate_commands(
                ast, n, plugins, cmds, filename, source, candidates);
        }
        else if (candidates_count(prev_candidates) > 0)
        {
            report_ambiguous_overloads(
                ast,
                ast->nodes[n].cmd.arglist,
                plugins,
                cmds,
                filename,
                source,
                rule_idx,
                prev_candidates);
        }
        else if (ctx.argcount < param_min)
        {
            report_available_commands(
                "Too few arguments to command.\n",
                ast,
                ast->nodes[n].cmd.arglist,
                plugins,
                cmds,
                ast->nodes[n].cmd.id,
                filename,
                source);
        }
        else if (ctx.argcount > param_max)
        {
            report_available_commands(
                "Too many arguments to command.\n",
                ast,
                ast->nodes[n].cmd.arglist,
                plugins,
                cmds,
                ast->nodes[n].cmd.id,
                filename,
                source);
        }
        else if (candidates_count(prev_candidates) == 0)
        {
            report_available_commands(
                "Parameter mismatch: No version of this command takes the "
                "argument types used here.\n",
                ast,
                ast->nodes[n].cmd.arglist,
                plugins,
                cmds,
                ast->nodes[n].cmd.id,
                filename,
                source);
        }

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
