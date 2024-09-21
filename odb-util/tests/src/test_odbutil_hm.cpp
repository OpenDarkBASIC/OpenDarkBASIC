#include <gmock/gmock.h>

extern "C" {
#include "odb-util/hash.h"
#include "odb-util/hm.h"
#include "odb-util/mem.h"
}

#define NAME         odbutil_hm
#define MIN_CAPACITY 128

using namespace ::testing;

static const uintptr_t KEY1 = 1111;
static const uintptr_t KEY2 = 2222;
static const uintptr_t KEY3 = 3333;
static const uintptr_t KEY4 = 4444;

HM_DECLARE_API(static, hm_test, uintptr_t, float, 32)
HM_DEFINE_API(hm_test, uintptr_t, float, 32);

struct NAME : Test
{
    virtual void
    SetUp()
    {
        hm_test_init(&hm);
    }

    virtual void
    TearDown()
    {
        hm_test_deinit(hm);
    }

    struct hm_test* hm;
};

TEST_F(NAME, null_hm_test_is_set)
{
    EXPECT_THAT(hm, IsNull());
    EXPECT_THAT(hm_test_count(hm), Eq(0));
    EXPECT_THAT(hm_test_capacity(hm), Eq(0));
}

TEST_F(NAME, deinit_null_hm_test_works)
{
    hm_test_deinit(hm);
}

TEST_F(NAME, insert_increases_slots_used)
{
    EXPECT_THAT(hm_test_count(hm), Eq(0));
    EXPECT_THAT(hm_test_insert_new(&hm, KEY1, 5.6f), Eq(0));
    EXPECT_THAT(hm_test_count(hm), Eq(1));
    EXPECT_THAT(hm_test_capacity(hm), Eq(MIN_CAPACITY));
}

TEST_F(NAME, erase_decreases_slots_used)
{
    EXPECT_THAT(hm_test_insert_new(&hm, KEY1, 5.6f), Eq(0));
    EXPECT_THAT(hm_test_erase(hm, KEY1), Pointee(5.6f));
    EXPECT_THAT(hm_test_count(hm), Eq(0));
    EXPECT_THAT(hm_test_capacity(hm), Eq(MIN_CAPACITY));
}

TEST_F(NAME, insert_same_key_twice_only_works_once)
{
    EXPECT_THAT(hm_test_insert_new(&hm, KEY1, 5.6f), Eq(0));
    EXPECT_THAT(hm_test_insert_new(&hm, KEY1, 7.6f), Eq(-1));
    EXPECT_THAT(hm_test_count(hm), Eq(1));
}

TEST_F(NAME, insert_or_get_returns_inserted_value)
{
    float  f = 0.0f;
    float* p = &f;
    EXPECT_THAT(hm_test_emplace_or_get(&hm, KEY1, &p), HM_NEW);
    *p = 5.6f;
    p = &f;
    EXPECT_THAT(hm_test_emplace_or_get(&hm, KEY1, &p), HM_EXISTS);
    EXPECT_THAT(f, Eq(0.0f));
    EXPECT_THAT(p, Pointee(5.6f));
    EXPECT_THAT(hm_test_count(hm), Eq(1));
}

TEST_F(NAME, erasing_same_key_twice_only_works_once)
{
    EXPECT_THAT(hm_test_insert_new(&hm, KEY1, 5.6f), Eq(0));
    EXPECT_THAT(hm_test_erase(hm, KEY1), Pointee(5.6f));
    EXPECT_THAT(hm_test_erase(hm, KEY1), IsNull());
    EXPECT_THAT(hm_test_count(hm), Eq(0));
}

TEST_F(NAME, insert_ab_erase_ba)
{
    EXPECT_THAT(hm_test_insert_new(&hm, KEY1, 5.6f), Eq(0));
    EXPECT_THAT(hm_test_insert_new(&hm, KEY2, 3.4f), Eq(0));
    EXPECT_THAT(hm_test_count(hm), Eq(2));
    EXPECT_THAT(hm_test_erase(hm, KEY2), Pointee(3.4f));
    EXPECT_THAT(hm_test_erase(hm, KEY1), Pointee(5.6f));
    EXPECT_THAT(hm_test_count(hm), Eq(0));
}

TEST_F(NAME, insert_ab_erase_ab)
{
    EXPECT_THAT(hm_test_insert_new(&hm, KEY1, 5.6f), Eq(0));
    EXPECT_THAT(hm_test_insert_new(&hm, KEY2, 3.4f), Eq(0));
    EXPECT_THAT(hm_test_count(hm), Eq(2));
    EXPECT_THAT(hm_test_erase(hm, KEY1), Pointee(5.6f));
    EXPECT_THAT(hm_test_erase(hm, KEY2), Pointee(3.4f));
    EXPECT_THAT(hm_test_count(hm), Eq(0));
}

TEST_F(NAME, insert_ab_find_ab)
{
    EXPECT_THAT(hm_test_insert_new(&hm, KEY1, 5.6f), Eq(0));
    EXPECT_THAT(hm_test_insert_new(&hm, KEY2, 3.4f), Eq(0));
    EXPECT_THAT(hm_test_find(hm, KEY1), Pointee(5.6f));
    EXPECT_THAT(hm_test_find(hm, KEY2), Pointee(3.4f));
}

TEST_F(NAME, insert_ab_erase_a_find_b)
{
    EXPECT_THAT(hm_test_insert_new(&hm, KEY1, 5.6f), Eq(0));
    EXPECT_THAT(hm_test_insert_new(&hm, KEY2, 3.4f), Eq(0));
    EXPECT_THAT(hm_test_erase(hm, KEY1), NotNull());
    EXPECT_THAT(hm_test_find(hm, KEY2), Pointee(3.4f));
}

TEST_F(NAME, insert_ab_erase_b_find_a)
{
    EXPECT_THAT(hm_test_insert_new(&hm, KEY1, 5.6f), Eq(0));
    EXPECT_THAT(hm_test_insert_new(&hm, KEY2, 3.4f), Eq(0));
    EXPECT_THAT(hm_test_erase(hm, KEY2), NotNull());
    EXPECT_THAT(hm_test_find(hm, KEY1), Pointee(5.6f));
}

TEST_F(NAME, rehash_test)
{
    uintptr_t key;
    float     value = 0;
    for (int i = 0; i != MIN_CAPACITY * 128; ++i, value += 1.5f)
    {
        key = i;
        ASSERT_THAT(hm_test_insert_new(&hm, key, value), Eq(0));
    }

    value = 0;
    for (int i = 0; i != MIN_CAPACITY * 128; ++i, value += 1.5f)
    {
        key = i;
        EXPECT_THAT(hm_test_erase(hm, key), Pointee(value)) << i;
    }
}

TEST_F(NAME, foreach_empty)
{
    uintptr_t key;
    float*    value;
    int       counter = 0;
    hm_for_each(hm, key, value)
    {
        counter++;
    }
    EXPECT_THAT(counter, Eq(0));
}

TEST_F(NAME, foreach)
{
    for (int i = 0; i != 16; ++i)
        hm_test_insert_new(&hm, i, float(i));

    hm_test_erase(hm, 5);
    hm_test_erase(hm, 8);
    hm_test_erase(hm, 14);
    hm_test_erase(hm, 3);
    hm_test_erase(hm, 11);
    hm_test_erase(hm, 6);

    for (int i = 16; i != 20; ++i)
        hm_test_insert_new(&hm, i, float(i));

    std::unordered_map<float, int> expected_values = {
        {0.0f, 0},
        {1.0f, 0},
        {2.0f, 0},
        {4.0f, 0},
        {7.0f, 0},
        {9.0f, 0},
        {10.0f, 0},
        {12.0f, 0},
        {13.0f, 0},
        {15.0f, 0},
        {16.0f, 0},
        {17.0f, 0},
        {18.0f, 0},
        {19.0f, 0},
    };

    uintptr_t key;
    float*    value;
    hm_for_each(hm, key, value)
    {
        expected_values[*value] += 1;
    }

    EXPECT_THAT(hm_test_count(hm), Eq(14));
    EXPECT_THAT(expected_values.size(), Eq(14));
    for (const auto& [k, v] : expected_values)
        EXPECT_THAT(v, Eq(1)) << k;
}
