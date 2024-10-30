#pragma once

#include "odb-util/config.h"
#include <stdint.h>

struct bm
{
    int      count;
    uint64_t data[1];
};

ODBUTIL_PUBLIC_API struct bm*
bm_create(int num_bits);

ODBUTIL_PUBLIC_API void
bm_destroy(struct bm* bm);

ODBUTIL_PUBLIC_API int
bm_grow(struct bm** bm, int num_bits);

ODBUTIL_PUBLIC_API void
bm_reset(struct bm* bm);

int
bm_set(struct bm* bm, int bit)
{
    int      idx = bit / 64;
    uint64_t mask = 1ULL << (bit & 0x3F);
    int      was_set = (bm->data[idx] & mask) != 0ULL;
    bm->data[idx] |= mask;
    return was_set;
}
