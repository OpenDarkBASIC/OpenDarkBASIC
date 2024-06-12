#include "odb-compiler/ast/ast.h"
#include "odb-compiler/sdk/used_cmds.h"
#include "odb-sdk/hash.h"
#include "odb-sdk/hm.h"
#include "odb-sdk/utf8.h"
#include "odb-sdk/vec.h"

VEC_DECLARE_API(spanlist, struct utf8_span, 32, static)
VEC_DEFINE_API(spanlist, struct utf8_span, 32)
VEC_DEFINE_API(cmd_ids, cmd_id, 32)

struct kvs
{
    const char*      text;
    struct spanlist* keys;
    struct cmd_ids*  values;
};

static hash32
kvs_hash(struct utf8_view key)
{
    return hash32_jenkins_oaat(key.data + key.off, key.len);
}
static int
kvs_alloc(struct kvs* kvs, int32_t capacity)
{
    kvs->text = NULL;

    spanlist_init(&kvs->keys);
    if (spanlist_resize(&kvs->keys, capacity) != 0)
        return -1;

    cmd_ids_init(&kvs->values);
    if (cmd_ids_resize(&kvs->values, capacity) != 0)
    {
        spanlist_deinit(kvs->keys);
        return log_oom(sizeof(enum type) * capacity, "kvs_alloc()");
    }

    return 0;
}
static void
kvs_free(struct kvs* kvs)
{
    cmd_ids_deinit(kvs->values);
    spanlist_deinit(kvs->keys);
}
static struct utf8_view
kvs_get_key(const struct kvs* kvs, int32_t slot)
{
    ODBSDK_DEBUG_ASSERT(kvs->text != NULL, (void)0);
    return utf8_span_view(kvs->text, kvs->keys->data[slot]);
}
static void
kvs_set_key(struct kvs* kvs, int32_t slot, struct utf8_view key)
{
    ODBSDK_DEBUG_ASSERT(kvs->text == NULL || kvs->text == key.data, (void)0);
    kvs->text = key.data;
    kvs->keys->data[slot] = utf8_view_span(kvs->text, key);
}
static cmd_id*
kvs_get_value(struct kvs* kvs, int32_t slot)
{
    return &kvs->values->data[slot];
}
static void
kvs_set_value(struct kvs* kvs, int32_t slot, cmd_id* value)
{
    kvs->values->data[slot] = *value;
}

HM_DECLARE_API_FULL(
    used_cmds_hm, hash32, struct utf8_view, cmd_id, 32, static, struct kvs)
HM_DEFINE_API_FULL(
    used_cmds_hm,
    hash32,
    struct utf8_view,
    cmd_id,
    32,
    kvs_hash,
    kvs_alloc,
    kvs_free,
    kvs_get_key,
    kvs_set_key,
    utf8_equal,
    kvs_get_value,
    kvs_set_value,
    128,
    70)

void
used_cmds_init(struct used_cmds_hm** hm)
{
    used_cmds_hm_init(hm);
}

int
used_cmds_append(struct used_cmds_hm** used, const struct ast* ast)
{
    ast_id n;
    for (n = 0; n != ast->node_count; n++)
    {
        if (ast->nodes[n].info.node_type != AST_COMMAND)
            continue;

        cmd_id cmd = ast->nodes[n].cmd.id;

    }

    return 0;
}

struct cmd_ids*
used_cmds_finalize(struct used_cmds_hm* hm)
{
    int32_t         front = 0;
    int32_t         back = hm->capacity - 1;
    struct cmd_ids* list = hm->kvs.values;
    while (1)
    {
        while (hm->hashes[front] != HM_SLOT_UNUSED
               && hm->hashes[front] != HM_SLOT_RIP && front < back)
            front++;
        while ((hm->hashes[back] == HM_SLOT_UNUSED
                || hm->hashes[back] == HM_SLOT_RIP)
               && front < back)
            back--;

        if (front >= back)
            break;

        list->data[front] = list->data[back];
#if defined(_DEBUG)
        list->data[back] = -1;
#endif
    }

    hm->kvs.values = &cmd_ids_null_vec;
    used_cmds_hm_deinit(hm);

    return list;
}
