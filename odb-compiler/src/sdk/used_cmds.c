#include "odb-compiler/ast/ast.h"
#include "odb-compiler/sdk/used_cmds.h"
#include "odb-util/hash.h"
#include "odb-util/hm.h"
#include "odb-util/utf8.h"
#include "odb-util/vec.h"

VEC_DECLARE_API(static, spanlist, struct utf8_span, 32)
VEC_DEFINE_API(spanlist, struct utf8_span, 32)
VEC_DEFINE_API(cmd_ids, cmd_id, 32)

struct used_cmds_hm_kvs
{
    struct cmd_ids* keys;
};

static hash32
kvs_hash(cmd_id key)
{
    return key;
}
static int
kvs_alloc(
    struct used_cmds_hm_kvs* kvs,
    struct used_cmds_hm_kvs* old_kvs,
    int32_t                  capacity)
{
    cmd_ids_init(&kvs->keys);
    return cmd_ids_resize(&kvs->keys, capacity);
}
static void
kvs_free_old(struct used_cmds_hm_kvs* kvs)
{
    cmd_ids_deinit(kvs->keys);
}
static void
kvs_free(struct used_cmds_hm_kvs* kvs)
{
    cmd_ids_deinit(kvs->keys);
}
cmd_id
kvs_get_key(const struct used_cmds_hm_kvs* kvs, int32_t slot)
{
    return kvs->keys->data[slot];
}
static int
kvs_set_key(struct used_cmds_hm_kvs* kvs, int32_t slot, cmd_id key)
{
    kvs->keys->data[slot] = key;
    return 0;
}
static int
kvs_keys_equal(cmd_id k1, cmd_id k2)
{
    return k1 == k2;
}
static char*
kvs_get_value(const struct used_cmds_hm_kvs* kvs, int32_t slot)
{
    return (void*)1;
}
static void
kvs_set_value(struct used_cmds_hm_kvs* kvs, int32_t slot, char* value)
{
}

HM_DECLARE_API_FULL(
    static, used_cmds_hm, hash32, cmd_id, char, 32, struct used_cmds_hm_kvs)
HM_DEFINE_API_FULL(
    used_cmds_hm,
    hash32,
    cmd_id,
    char,
    32,
    kvs_hash,
    kvs_alloc,
    kvs_free_old,
    kvs_free,
    kvs_get_key,
    kvs_set_key,
    kvs_keys_equal,
    kvs_get_value,
    kvs_set_value,
    128,
    70)

struct used_cmds
{
    struct used_cmds_hm hm;
};

void
used_cmds_init(struct used_cmds** hm)
{
    used_cmds_hm_init((struct used_cmds_hm**)hm);
}

int
used_cmds_append(struct used_cmds** used_cmds, const struct ast* ast)
{
    ast_id n;
    char*  c;
    for (n = 0; n != ast->count; n++)
        if (ast->nodes[n].info.node_type == AST_COMMAND)
            if (used_cmds_hm_emplace_or_get(
                    (struct used_cmds_hm**)used_cmds, ast->nodes[n].cmd.id, &c)
                == HM_OOM)
            {
                return -1;
            }

    return 0;
}

struct cmd_ids*
used_cmds_finalize(struct used_cmds* used_cmds)
{
    struct cmd_ids*      cmds;
    struct used_cmds_hm* hm = &used_cmds->hm;
    int32_t              front, back;

    if (hm == NULL)
        return NULL;

    front = 0;
    back = hm->capacity - 1;
    cmds = hm->kvs.keys;

    /* Move around values to fill gaps, so it can be used as a vector */
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

        cmds->data[front] = cmds->data[back];
        hm->hashes[front] = hm->hashes[back];
        hm->hashes[back] = HM_SLOT_UNUSED;
    }
    cmds->count = hm->count;

    hm->kvs.keys = NULL;
    used_cmds_hm_deinit(hm);

    return cmds;
}
