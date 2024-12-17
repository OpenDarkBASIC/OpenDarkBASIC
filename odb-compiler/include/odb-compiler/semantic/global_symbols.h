#pragma once

#include "odb-compiler/ast/ast.h"
#include "odb-util/utf8.h"

struct ospathc_list;

struct global
{
    /* Index into the list of TUs (translation units) of the AST in which this
     * symbol originated from + reference to original node. This is used for
     * error reporting duplicate definitions. */
    int tu_id;
    int original_node;

    /* References a node in the "global AST". */
    union type type;
};

struct global_symbols;

static inline void
global_symbols_init(struct global_symbols** global_symbols)
{
    *global_symbols = NULL;
}

ODBCOMPILER_PUBLIC_API void
global_symbols_deinit(struct global_symbols* global_symbols);

ODBCOMPILER_PUBLIC_API int
globals_add_declarations_from_ast(
    struct global_symbols**    global_symbols,
    struct ast**               globals,
    struct ast**               tus,
    int                        tu_id,
    const struct ospathc_list* filenames,
    const struct utf8*         sources);

ODBCOMPILER_PUBLIC_API const struct global*
global_symbols_find(const struct global_symbols* global_symbols, struct utf8_view key);

#if defined(ODBUTIL_MEM_DEBUGGING)
ODBCOMPILER_PUBLIC_API void
mem_acquire_globals(struct global_symbols* global_symbols);
ODBCOMPILER_PUBLIC_API void
mem_release_globals(struct global_symbols* global_symbols);
#else
#define mem_acquire_globals(global_symbols)
#define mem_release_globals(global_symbols)
#endif
