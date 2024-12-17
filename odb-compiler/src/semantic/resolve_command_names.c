#include "odb-compiler/ast/ast.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-compiler/semantic/type.h"
#include "odb-util/log.h"
#include "odb-util/mem.h"
#include "odb-util/vec.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>

static int
resolve_command_names(
    struct ast**               tus,
    int                        tu_count,
    int                        tu_id,
    struct mutex**             tu_mutexes,
    const struct ospathc_list* filenames,
    struct utf8*               sources,
    const struct plugin_list*  plugins,
    const struct cmd_list*     cmds,
    const struct globals*      globals)
{
    ast_id       n;
    struct utf8  cmd_name = empty_utf8();
    struct ast** astp = &tus[tu_id];
    struct ast*  ast = *astp;
    const char*  source = utf8_cstr(sources[tu_id]);

    for (n = 0; n != ast_count_unsafe(ast); ++n)
    {
        cmd_id           cmd;
        struct utf8_view cmd_view;
        union ast_node   node;

        if (ast_node_type(ast, n) != AST_COMMAND_NAME)
            continue;

        /* Commands are stored in the command list in upper case by
         * convention. For performance reasons we do the conversion to
         * upper here */
        cmd_view = utf8_span_view(source, ast->nodes[n].command_name.name);
        if (utf8_set(&cmd_name, cmd_view) != 0)
            goto fail;
        utf8_toupper(cmd_name);

        cmd = cmd_list_find(cmds, utf8_view(cmd_name));
        ODBUTIL_DEBUG_ASSERT(cmd > -1, (void)0);

        node = ast->nodes[n];
        ast->nodes[n].info.node_type = AST_COMMAND;
        ast->nodes[n].command._pad = -1;
        ast->nodes[n].command.arglist = node.command_name.arglist;
        ast->nodes[n].command.is_expr = node.command_name.is_expr;
        ast->nodes[n].command.id = cmd;
    }

    for (n = 0; n != ast_count_unsafe(ast); ++n)
        ODBUTIL_DEBUG_ASSERT(
            ast_node_type(ast, n) != AST_COMMAND_NAME, (void)0);

    utf8_deinit(cmd_name);
    return 0;

fail:
    utf8_deinit(cmd_name);
    return -1;
}

static const struct semantic_check* depends[] = {NULL};

const struct semantic_check semantic_resolve_command_names
    = {resolve_command_names, depends, "resolve_command_names"};
