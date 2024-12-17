#pragma once

#include "odb-compiler/ast/ast.h"
#include "odb-util/utf8.h"

struct ospathc_list;

struct global
{
    /* Index into the list of TUs (translation units) of the AST in which this
     * symbol is defined */
    int tu_id;
    /* Index of the AST node that defines this symbol.
     *   - In the case of functions, this will point either to the polymorphic
     *     function, or if the function is not a template, point to a AST_FUNC
     *   - In the case of UDTs, this will point to the udt_decl node that
     *     defines the type.
     */
    ast_id ast_node;
};

struct global_symbols;

static inline void
globals_init(struct global_symbols** table)
{
    *table = NULL;
}

ODBCOMPILER_PUBLIC_API void
global_symbols_deinit(struct global_symbols* table);

ODBCOMPILER_PUBLIC_API int
globals_add_declarations_from_ast(
    struct global_symbols**           table,
    struct ast**               tus,
    int                        tu_id,
    const struct ospathc_list* filenames,
    const struct utf8*         sources);

ODBCOMPILER_PUBLIC_API const struct global*
global_symbols_find(const struct global_symbols* table, struct utf8_view key);

#if defined(ODBUTIL_MEM_DEBUGGING)
ODBCOMPILER_PUBLIC_API void
mem_acquire_globals(struct global_symbols* table);
ODBCOMPILER_PUBLIC_API void
mem_release_globals(struct global_symbols* table);
#else
#define mem_acquire_globals(table)
#define mem_release_globals(table)
#endif
