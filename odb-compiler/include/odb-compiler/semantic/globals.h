#pragma once

#include "odb-compiler/ast/ast.h"
#include "odb-util/utf8.h"

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

struct globals;
struct db_source;

static inline void
globals_init(struct globals** table)
{
    *table = NULL;
}

ODBCOMPILER_PUBLIC_API void
globals_deinit(struct globals* table);

ODBCOMPILER_PUBLIC_API int
globals_add_declarations_from_ast(
    struct globals**   table,
    struct ast**            tus,
    int                     tu_id,
    const struct utf8*      filenames,
    const struct db_source* sources);

ODBCOMPILER_PUBLIC_API const struct global*
globals_find(const struct globals* table, struct utf8_view key);

#if defined(ODBUTIL_MEM_DEBUGGING)
ODBCOMPILER_PUBLIC_API void
mem_acquire_globals(struct globals* table);
ODBCOMPILER_PUBLIC_API void
mem_release_globals(struct globals* table);
#else
#define mem_acquire_globals(table)
#define mem_release_globals(table)
#endif
