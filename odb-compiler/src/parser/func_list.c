#include "odb-compiler/parser/func_list.h"
#include "odb-sdk/mem.h"

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
kvs_alloc(struct func_table_kvs* kvs, int32_t capacity)
{
    kvs->key_spans = mem_alloc(sizeof(*kvs->key_spans) * capacity);
    if (kvs->key_spans == NULL)
        return -1;

    return 0;
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
