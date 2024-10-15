#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-compiler/semantic/symbol_table.h"

static int
resolve_func_call(
    struct ast*                ast,
    const struct plugin_list*  plugins,
    const struct cmd_list*     cmds,
    const struct symbol_table* symbols,
    const char*                source_filename,
    struct db_source           source)
{
    ast_id n;
    for (n = 0; n != ast->node_count; ++n)
    {
        if (ast->nodes[n].info.node_type != AST_FUNC_OR_CONTAINER_REF)
            continue;

        ast_id identifier = ast->nodes[n].func_or_container_ref.identifier;
        struct utf8_view key = utf8_span_view(
            source.text.data, ast->nodes[identifier].identifier.name);
        const struct symbol_table_entry* entry
            = symbol_table_find(symbols, key);

        if (entry == NULL)
        {
            log_flc_err(
                source_filename,
                source.text.data,
                ast->nodes[n].info.location,
                "No function with this name exists.\n");
            log_excerpt_1(source.text.data, ast->nodes[n].info.location, "");
            return -1;
        }

        /* TODO: Type check arguments and return value */

        ast->nodes[n].info.node_type = AST_FUNC_CALL;
    }

    return 0;
}

static const struct semantic_check* depends[]
    = {&semantic_expand_constant_declarations, NULL};

const struct semantic_check semantic_resolve_func_or_container_refs
    = {resolve_func_call, depends};
