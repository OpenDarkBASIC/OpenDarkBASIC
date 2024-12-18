#pragma once

#include "odb-compiler/ast/ast.h"
#include "odb-util/utf8.h"

struct hm;
struct ospathc_list;

/* Global symbols and their definitions are stored in this structure. This
 * includes variables, functions, polymorphic functions, User-Defined Type
 * definitions, and array definitions. */
struct globals
{
    struct hm*  name_map;
    struct ast* ast;
    struct utf8 source;
};

/* An entry in the global symbol table name map */
struct global
{
    /* Index into the list of TUs (translation units) of the AST in which this
     * symbol originated from + reference to original node. This is used for
     * message reporting */
    int tu_id;
    ast_id ast_node;

    /* References a node in the "global AST". */
    union type type;
};

ODBCOMPILER_PUBLIC_API void
globals_init(struct globals* globals);

ODBCOMPILER_PUBLIC_API void
globals_deinit(struct globals* globals);

ODBCOMPILER_PUBLIC_API union type
globals_add_type(
    struct globals*   globals,
    ast_id            udt_decl,
    const struct ast* ast,
    struct ospathc    filename,
    const char*       source);

ODBCOMPILER_PUBLIC_API int
globals_add_from_ast(
    struct globals*            globals,
    struct ast**               tus,
    int                        tu_id,
    const struct ospathc_list* filenames,
    const struct utf8*         sources);

ODBCOMPILER_PUBLIC_API const struct global*
globals_find(const struct globals* globals, struct utf8_view name);

#if defined(ODBUTIL_MEM_DEBUGGING)
ODBCOMPILER_PUBLIC_API void
mem_acquire_globals(struct globals* globals);
ODBCOMPILER_PUBLIC_API void
mem_release_globals(struct globals* globals);
#else
#define mem_acquire_globals(globals)
#define mem_release_globals(globals)
#endif
