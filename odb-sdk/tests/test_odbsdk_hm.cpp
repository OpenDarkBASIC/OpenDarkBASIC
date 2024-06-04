#include "odb-sdk/hash.h"

#include <gmock/gmock.h>

extern "C" {
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

static hash32
shitty_hash(const void* data, int len)
{
    return 42;
}
static hash32
collide_with_shitty_hash(const void* data, int len)
{
    return MIN_CAPACITY + 42;
}
static hash32
collide_with_shitty_hash_second_probe(const void* data, int len)
{
    return MIN_CAPACITY + 45; // sequence would be 42, 43, 45, 48, ...
}

struct kvs
{
    char*  keys;
    float* values;
};

static hash32
test_hash(const char* key)
{
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
static const char*
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
static float
test_get_value(const struct kvs* kvs, int16_t slot)
{
    return kvs->values[slot];
}
static void
test_set_value(struct kvs* kvs, int16_t slot, float value)
{
    kvs->values[slot] = value;
}

#define NO_API
HM_DECLARE_API_FULL(hm_test, hash32, key, float, 16, NO_API, struct kvs)
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

class NAME : public Test
{
protected:
    struct hm* hm;

public:
    virtual void
    SetUp()
    {
        hm = hm_alloc(16, sizeof(float));
        ASSERT_THAT(hm, NotNull());
    }

    virtual void
    TearDown()
    {
        hm_free(hm);
    }
};

static int
hm_insert_value(struct hm* hm, const char* key, const float* value)
{
    float* hm_value;
    int    ret = hm_insert(hm, key, (void**)&hm_value);
    if (ret == 1)
        *hm_value = *value;
    return ret;
}

TEST_F(NAME, construct_sane_values)
{
    EXPECT_THAT(hm->table_count, Eq(MIN_CAPACITY));
    EXPECT_THAT(hm->key_size, Eq(16));
    EXPECT_THAT(hm->value_size, Eq(sizeof(float)));
    EXPECT_THAT(hm_count(hm), Eq(0));
    EXPECT_THAT(hm->storage, NotNull());
}

TEST_F(NAME, insert_increases_slots_used)
{
    float f = 5.6f;
    EXPECT_THAT(hm_count(hm), Eq(0));
    EXPECT_THAT(hm_insert_value(hm, KEY1, &f), Eq(1));
    EXPECT_THAT(hm_count(hm), Eq(1));
}

TEST_F(NAME, erase_decreases_slots_used)
{
    float f = 5.6f;
    EXPECT_THAT(hm_insert_value(hm, KEY1, &f), Eq(1));
    EXPECT_THAT(hm_count(hm), Eq(1));
    EXPECT_THAT(hm_erase(hm, KEY1), NotNull());
    EXPECT_THAT(hm_count(hm), Eq(0));
}

TEST_F(NAME, erase_returns_value)
{
    float f = 5.6f;
    EXPECT_THAT(hm_insert_value(hm, KEY1, &f), Eq(1));
    EXPECT_THAT(*(float*)hm_erase(hm, KEY1), FloatEq(f));
}

TEST_F(NAME, insert_same_key_twice_only_works_once)
{
    float f = 5.6f;
    EXPECT_THAT(hm_insert_value(hm, KEY1, &f), Eq(1));
    EXPECT_THAT(hm_insert_value(hm, KEY1, &f), Eq(0));
    EXPECT_THAT(hm_count(hm), Eq(1));
}

TEST_F(NAME, erasing_same_key_twice_only_works_once)
{
    float f = 5.6f;
    EXPECT_THAT(hm_insert_value(hm, KEY1, &f), Eq(1));
    EXPECT_THAT(hm_erase(hm, KEY1), NotNull());
    EXPECT_THAT(hm_erase(hm, KEY1), IsNull());
    EXPECT_THAT(hm_count(hm), Eq(0));
}

TEST_F(NAME, hash_collision_insert_ab_erase_ba)
{
    float a = 5.6f;
    float b = 3.4f;
    hm->hash = shitty_hash;
    EXPECT_THAT(hm_insert_value(hm, KEY1, &a), Eq(1));
    EXPECT_THAT(hm_insert_value(hm, KEY2, &b), Eq(1));
    EXPECT_THAT(hm_count(hm), Eq(2));
    EXPECT_THAT(*(float*)hm_erase(hm, KEY2), FloatEq(b));
    EXPECT_THAT(*(float*)hm_erase(hm, KEY1), FloatEq(a));
    EXPECT_THAT(hm_count(hm), Eq(0));
}

TEST_F(NAME, hash_collision_insert_ab_erase_ab)
{
    float a = 5.6f;
    float b = 3.4f;
    hm->hash = shitty_hash;
    EXPECT_THAT(hm_insert_value(hm, KEY1, &a), Eq(1));
    EXPECT_THAT(hm_insert_value(hm, KEY2, &b), Eq(1));
    EXPECT_THAT(hm_count(hm), Eq(2));
    EXPECT_THAT(*(float*)hm_erase(hm, KEY1), FloatEq(a));
    EXPECT_THAT(*(float*)hm_erase(hm, KEY2), FloatEq(b));
    EXPECT_THAT(hm_count(hm), Eq(0));
}

TEST_F(NAME, hash_collision_insert_ab_find_ab)
{
    float a = 5.6f;
    float b = 3.4f;
    hm->hash = shitty_hash;
    EXPECT_THAT(hm_insert_value(hm, KEY1, &a), Eq(1));
    EXPECT_THAT(hm_insert_value(hm, KEY2, &b), Eq(1));
    EXPECT_THAT(*(float*)hm_find(hm, KEY1), FloatEq(a));
    EXPECT_THAT(*(float*)hm_find(hm, KEY2), FloatEq(b));
}

TEST_F(NAME, hash_collision_insert_ab_erase_a_find_b)
{
    float a = 5.6f;
    float b = 3.4f;
    hm->hash = shitty_hash;
    EXPECT_THAT(hm_insert_value(hm, KEY1, &a), Eq(1));
    EXPECT_THAT(hm_insert_value(hm, KEY2, &b), Eq(1));
    EXPECT_THAT(hm_erase(hm, KEY1), NotNull());
    EXPECT_THAT(*(float*)hm_find(hm, KEY2), FloatEq(b));
}

TEST_F(NAME, hash_collision_insert_ab_erase_b_find_a)
{
    float a = 5.6f;
    float b = 3.4f;
    hm->hash = shitty_hash;
    EXPECT_THAT(hm_insert_value(hm, KEY1, &a), Eq(1));
    EXPECT_THAT(hm_insert_value(hm, KEY2, &b), Eq(1));
    EXPECT_THAT(hm_erase(hm, KEY2), NotNull());
    EXPECT_THAT(*(float*)hm_find(hm, KEY1), FloatEq(a));
}

TEST_F(NAME, hash_collision_insert_at_tombstone)
{
    float a = 5.6f;
    float b = 3.4f;
    hm->hash = shitty_hash;
    EXPECT_THAT(hm_insert_value(hm, KEY1, &a), Eq(1));
    EXPECT_THAT(hm_insert_value(hm, KEY2, &b), Eq(1));
    EXPECT_THAT(hm_count(hm), Eq(2));
    EXPECT_THAT(*(float*)hm_erase(hm, KEY1), FloatEq(a)); // creates tombstone
    EXPECT_THAT(hm_count(hm), Eq(1));
    EXPECT_THAT(
        hm_insert_value(hm, KEY1, &a),
        Eq(1)); // should insert at tombstone location
    EXPECT_THAT(*(float*)hm_erase(hm, KEY1), FloatEq(a));
    EXPECT_THAT(*(float*)hm_erase(hm, KEY2), FloatEq(b));
    EXPECT_THAT(hm_count(hm), Eq(0));
}

TEST_F(NAME, hash_collision_insert_at_tombstone_with_existing_key)
{
    float a = 5.6f;
    float b = 3.4f;
    hm->hash = shitty_hash;
    EXPECT_THAT(hm_insert_value(hm, KEY1, &a), Eq(1));
    EXPECT_THAT(hm_insert_value(hm, KEY2, &b), Eq(1));
    EXPECT_THAT(hm_count(hm), Eq(2));
    EXPECT_THAT(*(float*)hm_erase(hm, KEY1), FloatEq(a)); // creates tombstone
    EXPECT_THAT(hm_count(hm), Eq(1));
    EXPECT_THAT(hm_insert_value(hm, KEY2, &a), Eq(0));
    EXPECT_THAT(hm_count(hm), Eq(1));
}

TEST_F(NAME, remove_probing_sequence_scenario_1)
{
    float a = 5.6f;
    float b = 3.4f;
    // Creates a tombstone in the probing sequence to KEY2
    hm->hash = shitty_hash;
    hm_insert_value(hm, KEY1, &a);
    hm_insert_value(hm, KEY2, &b);
    hm_erase(hm, KEY1);

    // Inserts a different hash into where the tombstone is
    hm->hash = collide_with_shitty_hash;
    hm_insert_value(hm, KEY1, &a);
    hm->hash = shitty_hash;

    // Does this cut off access to KEY2?
    /*ASSERT_THAT(hm_find(hm, KEY2), NotNull());
    EXPECT_THAT(*(float*)hm_find(hm, KEY2), FloatEq(b));*/
    float* ret = (float*)hm_erase(hm, KEY2);
    ASSERT_THAT(ret, NotNull());
    EXPECT_THAT(*ret, FloatEq(b));
}

TEST_F(NAME, remove_probing_sequence_scenario_2)
{
    float a = 5.6f;
    float b = 3.4f;
    float c = 1.8f;
    float d = 8.7f;

    // First key is inserted directly, next 2 collide and are inserted along the
    // probing sequence
    hm->hash = shitty_hash;
    hm_insert_value(hm, KEY1, &a);
    hm_insert_value(hm, KEY2, &b);
    hm_insert_value(hm, KEY3, &c);

    // Insert a key with a different hash that collides with the slot of KEY3
    hm->hash = collide_with_shitty_hash_second_probe;
    hm_insert_value(hm, KEY4, &d);

    // Erase KEY3
    hm->hash = shitty_hash; // restore shitty hash
    hm_erase(hm, KEY3);

    // Does this cut off access to KEY4?
    hm->hash = collide_with_shitty_hash_second_probe;
    float* ret = (float*)hm_erase(hm, KEY4);
    ASSERT_THAT(ret, NotNull());
    EXPECT_THAT(*ret, FloatEq(d));
}

TEST_F(NAME, rehash_test)
{
    char  key[16];
    float value = 0;
    for (int i = 0; i != MIN_CAPACITY * 128; ++i, value += 1.5f)
    {
        memset(key, 0, sizeof key);
        sprintf(key, "%d", i);
        ASSERT_THAT(hm_insert_value(hm, key, &value), Eq(1));
    }

    value = 0;
    for (int i = 0; i != MIN_CAPACITY * 128; ++i, value += 1.5f)
    {
        memset(key, 0, sizeof key);
        sprintf(key, "%d", i);
        float* retvalue = (float*)hm_erase(hm, key);
        EXPECT_THAT(retvalue, NotNull());
        if (retvalue != NULL)
            EXPECT_THAT(*retvalue, FloatEq(value));
        else
            printf("hm_erase() returned NULL for key %s\n", key);
    }
}
