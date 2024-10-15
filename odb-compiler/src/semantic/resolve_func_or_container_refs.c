#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_ops.h"
#include "odb-compiler/parser/db_source.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-compiler/semantic/symbol_table.h"

static int
resolve_func_or_container_refs(
    struct ast*                asts,
    int                        asts_count,
    int                        asts_id,
    struct mutex**             asts_mutex,
    const char**               filenames,
    const struct db_source*    sources,
    const struct plugin_list*  plugins,
    const struct cmd_list*     cmds,
    const struct symbol_table* symbols)
{
    ast_id      n;
    struct ast* ast = &asts[asts_id];
    const char* filename = filenames[asts_id];
    const char* source = sources[asts_id].text.data;

    for (n = 0; n != ast->node_count; ++n)
    {
        if (ast->nodes[n].info.node_type != AST_FUNC_OR_CONTAINER_REF)
            continue;

        ast_id identifier = ast->nodes[n].func_or_container_ref.identifier;
        struct utf8_view key
            = utf8_span_view(source, ast->nodes[identifier].identifier.name);
        const struct symbol_table_entry* entry
            = symbol_table_find(symbols, key);

        if (entry == NULL)
        {
            log_flc_err(
                filename,
                source,
                ast->nodes[n].info.location,
                "No function with this name exists.\n");
            log_excerpt_1(source, ast->nodes[n].info.location, "");
            return -1;
        }

        /* TODO: Type check arguments and return value */

        ast->nodes[n].info.node_type = AST_FUNC_CALL;
    }

    return 0;
}

static const struct semantic_check* depends[] = {NULL};

const struct semantic_check semantic_resolve_func_or_container_refs
    = {resolve_func_or_container_refs, depends};
