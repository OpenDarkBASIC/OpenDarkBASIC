#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-compiler/semantic/symbol_table.h"
#include "odb-util/hash.h"
#include "odb-util/hm.h"
#include "odb-util/mem.h"

VEC_DEFINE_API(func_param_types_list, enum type, 8)

struct kvs_key_data
{
    utf8_idx count, capacity;
    char     data[1];
};

struct kvs
{
    struct utf8_span*          key_spans;
    struct kvs_key_data*       key_data;
    struct symbol_table_entry* values;
};

static hash32
kvs_hash(struct utf8_view key)
{
    // TODO: Could just use the offset/length here?
    return hash32_jenkins_oaat(key.data + key.off, key.len);
}

static int
kvs_alloc(struct kvs* kvs, struct kvs* old_kvs, int32_t capacity)
{
    static const int avg_func_name_len = 32;
    int              header_size = offsetof(struct kvs_key_data, data);
    int              data_size = sizeof(char) * avg_func_name_len * capacity;
    kvs->key_data
        = old_kvs ? old_kvs->key_data : mem_alloc(header_size + data_size);
    if (kvs->key_data == NULL)
        goto alloc_data_failed;
    kvs->key_data->count = 0;
    kvs->key_data->capacity = capacity;

    kvs->key_spans = mem_alloc(sizeof(*kvs->key_spans) * capacity);
    if (kvs->key_spans == NULL)
        goto alloc_spans_failed;

    kvs->values = mem_alloc(sizeof(*kvs->values) * capacity);
    if (kvs->values == NULL)
        goto alloc_values_failed;

    if (old_kvs != NULL)
        old_kvs->key_data = NULL; /* Take ownership of old string data */

    return 0;

alloc_values_failed:
    mem_free(kvs->key_spans);
alloc_spans_failed:
    if (old_kvs == NULL)
        mem_free(kvs->key_data);
alloc_data_failed:
    return -1;
}

static void
kvs_free(struct kvs* kvs)
{
    mem_free(kvs->key_data);
    mem_free(kvs->key_spans);
    mem_free(kvs->values);
}

static struct utf8_view
kvs_get_key(const struct kvs* kvs, utf8_idx idx)
{
    return utf8_span_view(kvs->key_data->data, kvs->key_spans[idx]);
}

static int
kvs_set_key(struct kvs* kvs, utf8_idx idx, struct utf8_view key)
{
    while (kvs->key_data->count + key.len > kvs->key_data->capacity)
    {
        int   header_size = offsetof(struct kvs_key_data, data);
        int   data_size = sizeof(char) * kvs->key_data->capacity * 2;
        void* new_mem = mem_realloc(kvs->key_data, header_size + data_size);
        if (new_mem == NULL)
            return log_oom(header_size + data_size, "kvs_set_key()");
        kvs->key_data = new_mem;
        kvs->key_data->capacity *= 2;
    }

    kvs->key_spans[idx].off = kvs->key_data->count;
    kvs->key_spans[idx].len = key.len;
    memcpy(
        kvs->key_data->data + kvs->key_data->count,
        key.data + key.off,
        key.len);
    kvs->key_data->count += key.len;

    return 0;
}

static int
kvs_keys_equal(struct utf8_view a, struct utf8_view b)
{
    return utf8_equal(a, b);
}

static struct symbol_table_entry*
kvs_get_value(const struct kvs* kvs, utf8_idx idx)
{
    return &kvs->values[idx];
}

static void
kvs_set_value(struct kvs* kvs, utf8_idx idx, struct symbol_table_entry* value)
{
    kvs->values[idx] = *value;
}

HM_DECLARE_API_FULL(
    static,
    hm,
    hash32,
    struct utf8_view,
    struct symbol_table_entry,
    32,
    struct kvs)
HM_DEFINE_API_FULL(
    hm,
    hash32,
    struct utf8_view,
    struct symbol_table_entry,
    32,
    kvs_hash,
    kvs_alloc,
    kvs_free,
    kvs_get_key,
    kvs_set_key,
    kvs_keys_equal,
    kvs_get_value,
    kvs_set_value,
    128,
    70)
struct symbol_table
{
    struct hm hm;
};

void
symbol_table_deinit(struct symbol_table* table)
{
    struct symbol_table_entry* entry;
    struct utf8_view           func_name;

    hm_for_each_full(&table->hm, func_name, entry, kvs_get_key, kvs_get_value)
    {
        func_param_types_list_deinit(entry->param_types);
        (void)func_name;
    }

    hm_deinit(&table->hm);
}

int
symbol_table_add_declarations_from_ast(
    struct symbol_table**  table,
    const struct ast*      ast,
    const struct db_source source)
{
    ast_id n;
    for (n = 0; n != ast->node_count; ++n)
    {
        if (ast->nodes[n].info.node_type != AST_FUNC)
            continue;

        ast_id           decl = ast->nodes[n].func.decl;
        ast_id           ident = ast->nodes[decl].func_decl.identifier;
        struct utf8_span span = ast->nodes[ident].identifier.name;
        struct utf8_view func_name = utf8_span_view(source.text.data, span);

        struct symbol_table_entry* entry;
        switch (hm_emplace_or_get((struct hm**)table, func_name, &entry))
        {
            case HM_OOM: return -1;
            case HM_EXISTS: return -1;

            case HM_NEW: {
                ast_id paramlist;

                func_param_types_list_init(&entry->param_types);
                for (paramlist = ast->nodes[decl].func_decl.paramlist;
                     paramlist > -1;
                     paramlist = ast->nodes[paramlist].paramlist.next)
                {
                    ast_id param_ident
                        = ast->nodes[paramlist].paramlist.identifier;
                    enum type param_type
                        = ast->nodes[param_ident].info.type_info;
                    if (func_param_types_list_push(
                            &entry->param_types, param_type)
                        != 0)
                    {
                        return -1;
                    }
                }

                /* TODO: entry->return_type = semantic_resolve_function_return_type(
                    ast, n, plugins, cmds, table, source_filename, source_text);*/
            }
            break;
        }
    }

    return 0;
}

const struct symbol_table_entry*
symbol_table_find(const struct symbol_table* table, struct utf8_view key)
{
    return hm_find(&table->hm, key);
}

void
mem_acquire_symbol_table(struct symbol_table* table)
{
    struct symbol_table_entry* entry;
    struct utf8_view           func_name;

    if (table == NULL)
        return;

    mem_acquire(
        table,
        offsetof(struct hm, hashes)
            + table->hm.capacity * sizeof(table->hm.hashes[0]));
    ODBUTIL_DEBUG_ASSERT(table->hm.kvs.key_data != NULL, (void)0);
    mem_acquire(
        table->hm.kvs.key_data,
        offsetof(struct kvs_key_data, data)
            + sizeof(table->hm.kvs.key_data->data[0])
                  * table->hm.kvs.key_data->capacity);
    ODBUTIL_DEBUG_ASSERT(table->hm.kvs.key_spans != NULL, (void)0);
    mem_acquire(
        table->hm.kvs.key_spans,
        sizeof(table->hm.kvs.key_spans[0]) * table->hm.capacity);
    ODBUTIL_DEBUG_ASSERT(table->hm.kvs.values != NULL, (void)0);
    mem_acquire(
        table->hm.kvs.values,
        sizeof(table->hm.kvs.values[0]) * table->hm.capacity);

    hm_for_each_full(&table->hm, func_name, entry, kvs_get_key, kvs_get_value)
    {
        if (entry == NULL)
            continue;
        mem_acquire(
            entry->param_types,
            offsetof(struct func_param_types_list, data)
                + sizeof(entry->param_types->data[0])
                      * entry->param_types->capacity);
        (void)func_name;
    }
}

void
mem_release_symbol_table(struct symbol_table* table)
{
    struct symbol_table_entry* entry;
    struct utf8_view           func_name;

    if (table == NULL)
        return;

    hm_for_each_full(&table->hm, func_name, entry, kvs_get_key, kvs_get_value)
    {
        mem_release(entry->param_types);
        (void)func_name;
    }

    mem_release(table->hm.kvs.values);
    mem_release(table->hm.kvs.key_spans);
    mem_release(table->hm.kvs.key_data);
    mem_release(table);
}
