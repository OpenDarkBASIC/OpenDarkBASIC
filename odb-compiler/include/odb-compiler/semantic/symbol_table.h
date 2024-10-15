#pragma once

#include "odb-compiler/config.h"
#include "odb-compiler/parser/db_source.h"
#include "odb-compiler/sdk/type.h"
#include "odb-util/vec.h"

VEC_DECLARE_API(ODBCOMPILER_PUBLIC_API, func_param_types_list, enum type, 8)

struct symbol_table_entry
{
    struct func_param_types_list* param_types;
    enum type                     return_type;
};

struct symbol_table;
struct ast;

static inline void
symbol_table_init(struct symbol_table** table)
{
    *table = NULL;
}

ODBCOMPILER_PUBLIC_API void
symbol_table_deinit(struct symbol_table* table);

ODBCOMPILER_PUBLIC_API int
symbol_table_add_declarations_from_ast(
    struct symbol_table**  table,
    const struct ast*      ast,
    const struct db_source source);

ODBCOMPILER_PUBLIC_API const struct symbol_table_entry*
symbol_table_find(const struct symbol_table* table, struct utf8_view key);

ODBCOMPILER_PUBLIC_API void
mem_acquire_symbol_table(struct symbol_table* table);

ODBCOMPILER_PUBLIC_API void
mem_release_symbol_table(struct symbol_table* table);
