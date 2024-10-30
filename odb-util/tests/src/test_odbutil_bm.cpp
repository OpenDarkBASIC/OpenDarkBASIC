#include <gmock/gmock.h>

extern "C" {
#include "odb-util/bm.h"
}

#define NAME odbutil_bm

using namespace ::testing;

struct NAME : Test
{
    struct bm* bm = nullptr;
};

TEST_F(NAME, count_is_correct)
{
    bm = bm_create(1);
    ASSERT_THAT(bm->count, Eq(1));
    bm_destroy(bm);

    bm = bm_create(64);
    ASSERT_THAT(bm->count, Eq(1));
    bm_destroy(bm);

    bm = bm_create(64);
    ASSERT_THAT(bm->count, Eq(1));
    bm_destroy(bm);

    bm = bm_create(65);
    ASSERT_THAT(bm->count, Eq(2));
    bm_destroy(bm);

    bm = bm_create(128);
    ASSERT_THAT(bm->count, Eq(2));
    bm_destroy(bm);

    bm = bm_create(192);
    ASSERT_THAT(bm->count, Eq(3));
    bm_destroy(bm);

    bm = bm_create(193);
    ASSERT_THAT(bm->count, Eq(4));
    bm_destroy(bm);
}

TEST_F(NAME, set_and_reset)
{
    bm = bm_create(128);
    ASSERT_THAT(bm_set(bm, 0), Eq(0));
    ASSERT_THAT(bm_set(bm, 0), Eq(1));
    ASSERT_THAT(bm_set(bm, 0), Eq(1));
    ASSERT_THAT(bm_set(bm, 63), Eq(0));
    ASSERT_THAT(bm_set(bm, 63), Eq(1));
    ASSERT_THAT(bm_set(bm, 63), Eq(1));
    ASSERT_THAT(bm_set(bm, 64), Eq(0));
    ASSERT_THAT(bm_set(bm, 64), Eq(1));
    ASSERT_THAT(bm_set(bm, 64), Eq(1));
    ASSERT_THAT(bm->data[0], Eq(0x8000000000000001ULL));
    ASSERT_THAT(bm->data[1], Eq(0x0000000000000001ULL));
    bm_reset(bm);
    ASSERT_THAT(bm->data[0], Eq(0ULL));
    ASSERT_THAT(bm->data[1], Eq(0ULL));
    ASSERT_THAT(bm_set(bm, 0), Eq(0));
    ASSERT_THAT(bm_set(bm, 0), Eq(1));
    ASSERT_THAT(bm_set(bm, 0), Eq(1));
    ASSERT_THAT(bm_set(bm, 63), Eq(0));
    ASSERT_THAT(bm_set(bm, 63), Eq(1));
    ASSERT_THAT(bm_set(bm, 63), Eq(1));
    ASSERT_THAT(bm_set(bm, 64), Eq(0));
    ASSERT_THAT(bm_set(bm, 64), Eq(1));
    ASSERT_THAT(bm_set(bm, 64), Eq(1));
    bm_destroy(bm);
}

