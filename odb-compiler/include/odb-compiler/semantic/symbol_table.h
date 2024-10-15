#pragma once

#include "odb-compiler/ast/ast.h"
#include "odb-util/utf8.h"

struct symbol_table_entry
{
    /* Index into the list of TUs (translation units) of the AST in which this
     * symbol is defined */
    int tu_id;
    /* Index of the AST node that defines this symbol */
    ast_id ast_node;
};

struct symbol_table;
struct db_source;

static inline void
symbol_table_init(struct symbol_table** table)
{
    *table = NULL;
}

ODBCOMPILER_PUBLIC_API void
symbol_table_deinit(struct symbol_table* table);

ODBCOMPILER_PUBLIC_API int
symbol_table_add_declarations_from_ast(
    struct symbol_table**   table,
    struct ast**            tus,
    int                     tu_id,
    const struct db_source* sources);

ODBCOMPILER_PUBLIC_API const struct symbol_table_entry*
symbol_table_find(const struct symbol_table* table, struct utf8_view key);

ODBCOMPILER_PUBLIC_API void
mem_acquire_symbol_table(struct symbol_table* table);

ODBCOMPILER_PUBLIC_API void
mem_release_symbol_table(struct symbol_table* table);
