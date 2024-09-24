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

        /* TODO: Type check arguments and return value */
        
        /* TODO: Change func_or_container_ref -> func_call */
    }

    return 0;
}

static const struct semantic_check* depends[]
    = {&semantic_expand_constant_declarations, NULL};

const struct semantic_check semantic_func_call_resolve
    = {resolve_func_call, depends};
