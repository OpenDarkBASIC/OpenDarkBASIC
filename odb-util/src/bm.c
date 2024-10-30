#include "odb-util/bm.h"
#include "odb-util/mem.h"
#include <stddef.h>
#include <string.h>

struct bm*
bm_create(int num_bits)
{
    mem_size   header = offsetof(struct bm, data);
    int        count = (num_bits + 63) / 64;
    struct bm* bm = mem_alloc(header + count * sizeof(uint64_t));
    if (bm == NULL)
        return NULL;
    memset(bm->data, 0, count * sizeof(uint64_t));
    bm->count = count;
    return bm;
}

void
bm_destroy(struct bm* bm)
{
    mem_free(bm);
}

int
bm_grow(struct bm** bm, int num_bits)
{
    struct bm* new_bm;
    mem_size   header = offsetof(struct bm, data);
    int        count = (num_bits + 63) / 64;

    if (count <= (*bm)->count)
        return 0;

    new_bm = mem_realloc(*bm, header + count * sizeof(uint64_t));
    if (new_bm == NULL)
        return -1;

    memset(
        &new_bm->data[new_bm->count],
        0,
        (count - new_bm->count) * sizeof(uint64_t));
    new_bm->count = count;
    *bm = new_bm;
    return 0;
}

void
bm_reset(struct bm* bm)
{
    memset(bm->data, 0, bm->count * sizeof(uint64_t));
}
