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
