#include <gmock/gmock.h>

extern "C" {
#include "odb-sdk/hash.h"
#include "odb-sdk/hm.h"
#include "odb-sdk/mem.h"
}

#define NAME         odbsdk_hm
#define MIN_CAPACITY 64

using namespace ::testing;

static const char KEY1[16] = "KEY1";
static const char KEY2[16] = "KEY2";
static const char KEY3[16] = "KEY3";
static const char KEY4[16] = "KEY4";

enum hash_mode
{
    NORMAL,
    SHITTY,
    COLLIDE_WITH_SHITTY_HASH,
    COLLIDE_WITH_SHITTY_HASH_SECOND_PROBE
} hash_mode;

struct kvs
{
    char*  keys;
    float* values;
};

static hash32
test_hash(const char* key)
{
    switch (hash_mode)
    {
        case NORMAL: break;
        case SHITTY: return 42;
        case COLLIDE_WITH_SHITTY_HASH: return MIN_CAPACITY + 42;
        case COLLIDE_WITH_SHITTY_HASH_SECOND_PROBE:
            return MIN_CAPACITY + 45; // sequence would be 42, 43, 45, 48, ...
    }
    return hash32_jenkins_oaat(key, 4);
}
static int
test_storage_alloc(struct kvs* kvs, int16_t capacity)
{
    kvs->keys = (char*)mem_alloc(sizeof(char) * capacity * 16);
    kvs->values = (float*)mem_alloc(sizeof(*kvs->values) * capacity);
    return 0;
}
static void
test_storage_free(struct kvs* kvs)
{
    mem_free(kvs->values);
    mem_free(kvs->keys);
}
static char*
test_get_key(const struct kvs* kvs, int16_t slot)
{
    return &kvs->keys[slot * 16];
}
static void
test_set_key(struct kvs* kvs, int16_t slot, const char* key)
{
    memcpy(&kvs->keys[slot * 16], key, 16);
}
static int
test_keys_equal(const char* k1, const char* k2)
{
    return memcmp(k1, k2, 16) == 0;
}
static float*
test_get_value(const struct kvs* kvs, int16_t slot)
{
    return &kvs->values[slot];
}
static void
test_set_value(struct kvs* kvs, int16_t slot, const float* value)
{
    kvs->values[slot] = *value;
}

#define NO_API
HM_DECLARE_API_FULL(hm_test, hash32, char*, float, 16, NO_API, struct kvs)
HM_DEFINE_API_FULL(
    hm_test,
    hash32,
    char*,
    float,
    16,
    test_hash,
    test_storage_alloc,
    test_storage_free,
    test_get_key,
    test_set_key,
    test_keys_equal,
    test_get_value,
    test_set_value,
    MIN_CAPACITY,
    70);

struct NAME : Test
{
    virtual void
    SetUp()
    {
        hash_mode = NORMAL;
        hm_test_init(&hm);
    }

    virtual void
    TearDown()
    {
        hm_test_deinit(hm);
    }

    struct hm_test* hm;
};

static int
hm_test_insert_new(struct hm* hm, const char* key, const float* value)
{
    float* hm_value;
    int    ret = hm_insert(hm, key, (void**)&hm_value);
    if (ret == 1)
        *hm_value = *value;
    return ret;
}

TEST_F(NAME, null_hm_is_set)
{
    EXPECT_THAT(hm->count, Eq(0));
    EXPECT_THAT(hm->capacity, Eq(0));
    EXPECT_THAT(hm->kvs.keys, IsNull());
    EXPECT_THAT(hm->kvs.values, IsNull());
}

TEST_F(NAME, insert_increases_slots_used)
{
    EXPECT_THAT(hm->count, Eq(0));
    EXPECT_THAT(hm_test_insert_new(&hm, KEY1, 5.6f), Eq(0));
    EXPECT_THAT(hm->count, Eq(1));
    EXPECT_THAT(hm->capacity, Eq(MIN_CAPACITY));
}

TEST_F(NAME, erase_decreases_slots_used)
{
    EXPECT_THAT(hm_test_insert_new(&hm, KEY1, 5.6f), Eq(0));
    EXPECT_THAT(hm_test_erase(hm, KEY1), Pointee(5.6f));
    EXPECT_THAT(hm->count, Eq(0));
    EXPECT_THAT(hm->capacity, Eq(MIN_CAPACITY));
}

TEST_F(NAME, insert_same_key_twice_only_works_once)
{
    float f = 5.6f;
    EXPECT_THAT(hm_test_insert_new(&hm, KEY1, 5.6f), Eq(0));
    EXPECT_THAT(hm_test_insert_new(&hm, KEY1, 7.6f), Eq(-1));
    EXPECT_THAT(hm->count, Eq(1));
}

TEST_F(NAME, insert_or_get_returns_inserted_value)
{
    float f = 5.6f;
    EXPECT_THAT(hm_test_insert_or_get(&hm, KEY1, 5.6f), Pointee(5.6f));
    EXPECT_THAT(hm_test_insert_or_get(&hm, KEY1, 7.6f), Pointee(5.6f));
    EXPECT_THAT(hm->count, Eq(1));
}

TEST_F(NAME, erasing_same_key_twice_only_works_once)
{
    float f = 5.6f;
    EXPECT_THAT(hm_test_insert_new(&hm, KEY1, 5.6f), Eq(0));
    EXPECT_THAT(hm_test_erase(hm, KEY1), Pointee(5.6f));
    EXPECT_THAT(hm_test_erase(hm, KEY1), IsNull());
    EXPECT_THAT(hm->count, Eq(0));
}

TEST_F(NAME, hash_collision_insert_ab_erase_ba)
{
    float a = 5.6f;
    float b = 3.4f;
    hash_mode = SHITTY;
    EXPECT_THAT(hm_test_insert_new(&hm, KEY1, 5.6f), Eq(0));
    EXPECT_THAT(hm_test_insert_new(&hm, KEY2, 3.4f), Eq(0));
    EXPECT_THAT(hm->count, Eq(2));
    EXPECT_THAT(hm_test_erase(hm, KEY2), Pointee(b));
    EXPECT_THAT(hm_test_erase(hm, KEY1), Pointee(a));
    EXPECT_THAT(hm->count, Eq(0));
}

TEST_F(NAME, hash_collision_insert_ab_erase_ab)
{
    float a = 5.6f;
    float b = 3.4f;
    hash_mode = SHITTY;
    EXPECT_THAT(hm_test_insert_new(&hm, KEY1, 5.6f), Eq(0));
    EXPECT_THAT(hm_test_insert_new(&hm, KEY2, 3.4f), Eq(0));
    EXPECT_THAT(hm->count, Eq(2));
    EXPECT_THAT(hm_test_erase(hm, KEY1), Pointee(5.6f));
    EXPECT_THAT(hm_test_erase(hm, KEY2), Pointee(3.4f));
    EXPECT_THAT(hm->count, Eq(0));
}

TEST_F(NAME, hash_collision_insert_ab_find_ab)
{
    hash_mode = SHITTY;
    EXPECT_THAT(hm_test_insert_new(&hm, KEY1, 5.6f), Eq(0));
    EXPECT_THAT(hm_test_insert_new(&hm, KEY2, 3.4f), Eq(0));
    EXPECT_THAT(hm_test_find(hm, KEY1), Pointee(5.6f));
    EXPECT_THAT(hm_test_find(hm, KEY2), Pointee(3.4f));
}

TEST_F(NAME, hash_collision_insert_ab_erase_a_find_b)
{
    float a = 5.6f;
    float b = 3.4f;
    hash_mode = SHITTY;
    EXPECT_THAT(hm_test_insert_new(&hm, KEY1, 5.6f), Eq(0));
    EXPECT_THAT(hm_test_insert_new(&hm, KEY2, 3.4f), Eq(0));
    EXPECT_THAT(hm_test_erase(hm, KEY1), NotNull());
    EXPECT_THAT(hm_test_find(hm, KEY2), Pointee(3.4f));
}

TEST_F(NAME, hash_collision_insert_ab_erase_b_find_a)
{
    float a = 5.6f;
    float b = 3.4f;
    hash_mode = SHITTY;
    EXPECT_THAT(hm_test_insert_new(&hm, KEY1, 5.6f), Eq(0));
    EXPECT_THAT(hm_test_insert_new(&hm, KEY2, 3.4f), Eq(0));
    EXPECT_THAT(hm_test_erase(hm, KEY2), NotNull());
    EXPECT_THAT(hm_test_find(hm, KEY1), Pointee(5.6f));
}

TEST_F(NAME, hash_collision_insert_at_tombstone)
{
    float a = 5.6f;
    float b = 3.4f;
    hash_mode = SHITTY;
    EXPECT_THAT(hm_test_insert_new(&hm, KEY1, 5.6f), Eq(0));
    EXPECT_THAT(hm_test_insert_new(&hm, KEY2, 3.4f), Eq(0));
    EXPECT_THAT(hm->count, Eq(2));
    EXPECT_THAT(hm_test_erase(hm, KEY1), Pointee(5.6f)); // creates tombstone
    EXPECT_THAT(hm->count, Eq(1));
    EXPECT_THAT(
        hm_test_insert_new(&hm, KEY1, 5.6f),
        Eq(0)); // should insert at tombstone location
    EXPECT_THAT(hm_test_erase(hm, KEY1), Pointee(5.6f));
    EXPECT_THAT(hm_test_erase(hm, KEY2), Pointee(3.4f));
    EXPECT_THAT(hm->count, Eq(0));
}

TEST_F(NAME, hash_collision_insert_at_tombstone_with_existing_key)
{
    float a = 5.6f;
    float b = 3.4f;
    hash_mode = SHITTY;
    EXPECT_THAT(hm_test_insert_new(&hm, KEY1, 5.6f), Eq(0));
    EXPECT_THAT(hm_test_insert_new(&hm, KEY2, 3.4f), Eq(0));
    EXPECT_THAT(hm->count, Eq(2));
    EXPECT_THAT(hm_test_erase(hm, KEY1), Pointee(5.6f)); // creates tombstone
    EXPECT_THAT(hm->count, Eq(1));
    EXPECT_THAT(hm_test_insert_new(&hm, KEY2, 5.6f), Eq(-1));
    EXPECT_THAT(hm->count, Eq(1));
}

TEST_F(NAME, remove_probing_sequence_scenario_1)
{
    float a = 5.6f;
    float b = 3.4f;
    // Creates a tombstone in the probing sequence to KEY2
    hash_mode = SHITTY;
    hm_test_insert_new(&hm, KEY1, 5.6f);
    hm_test_insert_new(&hm, KEY2, 3.4f);
    hm_test_erase(hm, KEY1);

    // Inserts a different hash into where the tombstone is
    hash_mode = COLLIDE_WITH_SHITTY_HASH;
    hm_test_insert_new(&hm, KEY1, 5.6f);
    hash_mode = SHITTY;

    // Does this cut off access to KEY2?
    /*ASSERT_THAT(hm_test_find(hm, KEY2), NotNull());
    EXPECT_THAT(hm_test_find(hm, KEY2), Pointee(3.4f));*/
    EXPECT_THAT(hm_test_erase(hm, KEY2), Pointee(3.4f));
}

TEST_F(NAME, remove_probing_sequence_scenario_2)
{
    // First key is inserted directly, next 2 collide and are inserted along the
    // probing sequence
    hash_mode = SHITTY;
    hm_test_insert_new(&hm, KEY1, 5.6f);
    hm_test_insert_new(&hm, KEY2, 3.4f);
    hm_test_insert_new(&hm, KEY3, 1.8f);

    // Insert a key with a different hash that collides with the slot of KEY3
    hash_mode = COLLIDE_WITH_SHITTY_HASH_SECOND_PROBE;
    hm_test_insert_new(&hm, KEY4, 8.7f);

    // Erase KEY3
    hash_mode = SHITTY; // restore shitty hash
    hm_test_erase(hm, KEY3);

    // Does this cut off access to KEY4?
    hash_mode = COLLIDE_WITH_SHITTY_HASH_SECOND_PROBE;
    EXPECT_THAT(hm_test_erase(hm, KEY4), Pointee(8.7f));
}

TEST_F(NAME, rehash_test)
{
    char  key[16];
    float value = 0;
    for (int i = 0; i != MIN_CAPACITY * 128; ++i, value += 1.5f)
    {
        memset(key, 0, sizeof key);
        sprintf(key, "%d", i);
        ASSERT_THAT(hm_test_insert_new(&hm, key, value), Eq(0));
    }

    value = 0;
    for (int i = 0; i != MIN_CAPACITY * 128; ++i, value += 1.5f)
    {
        memset(key, 0, sizeof key);
        sprintf(key, "%d", i);
        EXPECT_THAT(hm_test_erase(hm, key), Pointee(value));
    }
}
