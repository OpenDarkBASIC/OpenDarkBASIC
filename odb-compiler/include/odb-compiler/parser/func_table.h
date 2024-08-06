#pragma once

#include "odb-compiler/config.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-sdk/hm.h"

struct func_table_entry
{
    struct param_types_list* param_types;
    enum type                return_type;
};

struct func_table_kvs_key_data;
struct func_table_kvs
{
    struct utf8_span*               key_spans;
    struct func_table_kvs_key_data* key_data;
    struct func_table_entry*        values;
};

HM_DECLARE_API_FULL(
    ODBCOMPILER_PUBLIC_API,
    func_table,
    hash32,
    struct utf8_view,
    struct func_table_entry,
    32,
    struct func_table_kvs)
