#include "odb-compiler/ast/ast.h"
#include "odb-compiler/parser/func_table.h"
#include "odb-sdk/mem.h"

VEC_DEFINE_API(func_param_types_list, enum type, 8)

struct func_table_kvs_key_data
{
    utf8_idx count, capacity;
    char     data[1];
};

static hash32
kvs_hash(struct utf8_view key)
{
    return hash32_jenkins_oaat(key.data + key.off, key.len);
}

static int
kvs_alloc(
    struct func_table_kvs* kvs,
    struct func_table_kvs* old_kvs,
    int32_t                capacity)
{
    static const int avg_func_name_len = 32;
    int header_size = offsetof(struct func_table_kvs_key_data, data);
    int data_size = sizeof(char) * avg_func_name_len * capacity;
    kvs->key_data = old_kvs->key_data ? old_kvs->key_data
                                      : mem_alloc(header_size + data_size);
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

    old_kvs->key_data = NULL; /* Take ownership of old string data */

    return 0;

alloc_values_failed:
    mem_free(kvs->key_spans);
alloc_spans_failed:
    if (old_kvs->key_data == NULL)
        mem_free(kvs->key_data);
alloc_data_failed:
    return -1;
}

static void
kvs_free(struct func_table_kvs* kvs)
{
    if (kvs->key_data != NULL)
        mem_free(kvs->key_data);
    mem_free(kvs->key_spans);
    mem_free(kvs->values);
}

static struct utf8_view
kvs_get_key(const struct func_table_kvs* kvs, utf8_idx idx)
{
    return utf8_span_view(kvs->key_data->data, kvs->key_spans[idx]);
}

static int
kvs_set_key(struct func_table_kvs* kvs, utf8_idx idx, struct utf8_view key)
{
    while (kvs->key_data->count + key.len > kvs->key_data->capacity)
    {
        int   header_size = offsetof(struct func_table_kvs_key_data, data);
        int   data_size = sizeof(char) * kvs->key_data->capacity * 2;
        void* new_mem = mem_realloc(kvs->key_data, header_size + data_size);
        if (new_mem == NULL)
            return log_oom(header_size + data_size, "kvs_set_key()");
    }

    return -1;
}

static int
kvs_keys_equal(struct utf8_view a, struct utf8_view b)
{
    ODBSDK_DEBUG_ASSERT(a.data == b.data, (void)0);
    return a.off == b.off && a.len == b.len;
}

static struct func_table_entry*
kvs_get_value(const struct func_table_kvs* kvs, utf8_idx idx)
{
    return &kvs->values[idx];
}

static void
kvs_set_value(
    struct func_table_kvs* kvs, utf8_idx idx, struct func_table_entry* value)
{
    kvs->values[idx] = *value;
}

HM_DEFINE_API_FULL(
    func_table,
    hash32,
    struct utf8_view,
    struct func_table_entry,
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

int
func_table_add_declarations_from_ast(
    struct func_table*     table,
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

        struct func_table_entry* entry;
        switch (func_table_emplace_or_get(&table, func_name, &entry))
        {
            case HM_OOM: return -1;

            case HM_EXISTS: {
                return -1;
            }

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

                entry->return_type = ast->nodes[n].info.type_info;
            }
            break;
        }
    }

    return 0;
}
