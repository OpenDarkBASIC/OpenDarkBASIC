#pragma once

#include "odb-compiler/config.h"
#include "odb-compiler/parser/db_source.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-sdk/hm.h"

VEC_DECLARE_API(ODBCOMPILER_PUBLIC_API, func_param_types_list, enum type, 8)

struct symbol_table_entry
{
    struct func_param_types_list* param_types;
    enum type                     return_type;
};

struct symbol_table_kvs_key_data;
struct symbol_table_kvs
{
    struct utf8_span*                 key_spans;
    struct symbol_table_kvs_key_data* key_data;
    struct symbol_table_entry*        values;
};

HM_DECLARE_API_FULL(
    ODBCOMPILER_PUBLIC_API,
    symbol_table,
    hash32,
    struct utf8_view,
    struct symbol_table_entry,
    32,
    struct symbol_table_kvs)

ODBCOMPILER_PUBLIC_API int
symbol_table_add_declarations_from_ast(
    struct symbol_table**  table,
    const struct ast*      ast,
    const struct db_source source);
