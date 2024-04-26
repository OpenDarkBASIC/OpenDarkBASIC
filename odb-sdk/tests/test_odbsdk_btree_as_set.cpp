extern "C" {
#include "odb-sdk/btree.h"
}

#include "gmock/gmock.h"

#define NAME vh_btree_set

using namespace testing;

struct data_t
{
    float x, y, z;
};

TEST(NAME, init_sets_correct_values)
{
    struct btree btree;
    btree.count = 4;
    btree.capacity = 56;
    btree.data = (uint8_t*)4783;
    btree.value_size = 283;

    btree_init(&btree, 0);
    EXPECT_THAT(btree.count, Eq(0u));
    EXPECT_THAT(btree.capacity, Eq(0u));
    EXPECT_THAT(btree.count, Eq(0u));
    EXPECT_THAT(btree.data, IsNull());
    EXPECT_THAT(btree.value_size, Eq(0));
}

TEST(NAME, create_initializes_btree)
{
    struct btree* btree = btree_alloc(0);
    ASSERT_THAT(btree, NotNull());
    EXPECT_THAT(btree_capacity(btree), Eq(0u));
    EXPECT_THAT(btree_count(btree), Eq(0u));
    EXPECT_THAT(btree->data, IsNull());
    EXPECT_THAT(btree->value_size, Eq(0));
    btree_free(btree);
}

TEST(NAME, insertion_forwards)
{
    struct btree btree;
    btree_init(&btree, 0);

    ASSERT_THAT(btree_insert_new(&btree, 0, NULL), Eq(1));  ASSERT_THAT(btree_count(&btree), Eq(1u));
    ASSERT_THAT(btree_insert_new(&btree, 1, NULL), Eq(1));  ASSERT_THAT(btree_count(&btree), Eq(2u));
    ASSERT_THAT(btree_insert_new(&btree, 2, NULL), Eq(1));  ASSERT_THAT(btree_count(&btree), Eq(3u));
    ASSERT_THAT(btree_insert_new(&btree, 3, NULL), Eq(1));  ASSERT_THAT(btree_count(&btree), Eq(4u));
    ASSERT_THAT(btree_insert_new(&btree, 4, NULL), Eq(1));  ASSERT_THAT(btree_count(&btree), Eq(5u));

    EXPECT_THAT(btree_key_exists(&btree, 0), IsTrue());
    EXPECT_THAT(btree_key_exists(&btree, 1), IsTrue());
    EXPECT_THAT(btree_key_exists(&btree, 2), IsTrue());
    EXPECT_THAT(btree_key_exists(&btree, 3), IsTrue());
    EXPECT_THAT(btree_key_exists(&btree, 4), IsTrue());
    EXPECT_THAT(btree_key_exists(&btree, 5), IsFalse());

    btree_deinit(&btree);
}

TEST(NAME, clear_keeps_underlying_buffer)
{
    struct btree btree;
    btree_init(&btree, 0);

    btree_insert_new(&btree, 0, NULL);
    btree_insert_new(&btree, 1, NULL);
    btree_insert_new(&btree, 2, NULL);

    // this should delete all entries but keep the underlying buffer
    btree_clear(&btree);

    EXPECT_THAT(btree_count(&btree), Eq(0u));
    EXPECT_THAT(btree.data, NotNull());
    EXPECT_THAT(btree_capacity(&btree), Ne(0u));

    btree_deinit(&btree);
}

TEST(NAME, compact_when_no_buffer_is_allocated_doesnt_crash)
{
    struct btree btree;
    btree_init(&btree, 0);
    btree_compact(&btree);
    btree_deinit(&btree);
}

TEST(NAME, compact_reduces_capacity_and_keeps_elements_in_tact)
{
    struct btree btree;
    btree_init(&btree, 0);

    for (int i = 0; i != ODBSDK_BTREE_MIN_CAPACITY * 3; ++i)
        ASSERT_THAT(btree_insert_new(&btree, i, NULL), Eq(1));
    for (int i = 0; i != ODBSDK_BTREE_MIN_CAPACITY; ++i)
        btree_erase(&btree, i);

    btree_size old_capacity = btree_capacity(&btree);
    btree_compact(&btree);
    EXPECT_THAT(btree_capacity(&btree), Lt(old_capacity));
    EXPECT_THAT(btree_count(&btree), Eq(ODBSDK_BTREE_MIN_CAPACITY * 2));
    EXPECT_THAT(btree_capacity(&btree), Eq(ODBSDK_BTREE_MIN_CAPACITY * 2));
    EXPECT_THAT(btree.data, NotNull());

    btree_deinit(&btree);
}

TEST(NAME, clear_and_compact_deletes_underlying_buffer)
{
    struct btree btree;
    btree_init(&btree, 0);

    btree_insert_new(&btree, 0, NULL);
    btree_insert_new(&btree, 1, NULL);
    btree_insert_new(&btree, 2, NULL);

    // this should delete all entries + free the underlying buffer
    btree_clear(&btree);
    btree_compact(&btree);

    ASSERT_THAT(btree_count(&btree), Eq(0u));
    ASSERT_THAT(btree.data, IsNull());
    ASSERT_THAT(btree_capacity(&btree), Eq(0u));

    btree_deinit(&btree);
}

TEST(NAME, insert_new_existing_keys_fails)
{
    struct btree btree;
    btree_init(&btree, 0);

    EXPECT_THAT(btree_insert_new(&btree, 0, NULL), Eq(1));
    EXPECT_THAT(btree_insert_new(&btree, 0, NULL), Eq(0));

    btree_deinit(&btree);
}

TEST(NAME, key_exists_returns_false_if_key_doesnt_exist)
{
    struct btree btree;
    btree_init(&btree, 0);

    btree_insert_new(&btree, 3, NULL);
    btree_insert_new(&btree, 8, NULL);
    btree_insert_new(&btree, 2, NULL);

    EXPECT_THAT(btree_key_exists(&btree, 4), IsFalse());

    btree_deinit(&btree);
}

TEST(NAME, key_exists_returns_true_if_key_does_exist)
{
    struct btree btree;
    btree_init(&btree, 0);

    btree_insert_new(&btree, 3, NULL);
    btree_insert_new(&btree, 8, NULL);
    btree_insert_new(&btree, 2, NULL);

    EXPECT_THAT(btree_key_exists(&btree, 3), IsTrue());

    btree_deinit(&btree);
}

TEST(NAME, erase_by_key)
{
    struct btree btree;
    btree_init(&btree, 0);

    int a=56, b=45, c=18, d=27, e=84;
    btree_insert_new(&btree, 0, NULL);
    btree_insert_new(&btree, 1, NULL);
    btree_insert_new(&btree, 2, NULL);
    btree_insert_new(&btree, 3, NULL);
    btree_insert_new(&btree, 4, NULL);

    EXPECT_THAT(btree_erase(&btree, 2), Eq(1));

    // 4
    EXPECT_THAT(btree_key_exists(&btree, 0), IsTrue());
    EXPECT_THAT(btree_key_exists(&btree, 1), IsTrue());
    EXPECT_THAT(btree_key_exists(&btree, 2), IsFalse());
    EXPECT_THAT(btree_key_exists(&btree, 3), IsTrue());
    EXPECT_THAT(btree_key_exists(&btree, 4), IsTrue());

    EXPECT_THAT(btree_erase(&btree, 4), Eq(1));

    // 3
    EXPECT_THAT(btree_key_exists(&btree, 0), IsTrue());
    EXPECT_THAT(btree_key_exists(&btree, 1), IsTrue());
    EXPECT_THAT(btree_key_exists(&btree, 2), IsFalse());
    EXPECT_THAT(btree_key_exists(&btree, 3), IsTrue());
    EXPECT_THAT(btree_key_exists(&btree, 4), IsFalse());

    EXPECT_THAT(btree_erase(&btree, 0), Eq(1));

    // 2
    EXPECT_THAT(btree_key_exists(&btree, 0), IsFalse());
    EXPECT_THAT(btree_key_exists(&btree, 1), IsTrue());
    EXPECT_THAT(btree_key_exists(&btree, 2), IsFalse());
    EXPECT_THAT(btree_key_exists(&btree, 3), IsTrue());
    EXPECT_THAT(btree_key_exists(&btree, 4), IsFalse());

    EXPECT_THAT(btree_erase(&btree, 1), Eq(1));

    // 1
    EXPECT_THAT(btree_key_exists(&btree, 0), IsFalse());
    EXPECT_THAT(btree_key_exists(&btree, 1), IsFalse());
    EXPECT_THAT(btree_key_exists(&btree, 2), IsFalse());
    EXPECT_THAT(btree_key_exists(&btree, 3), IsTrue());
    EXPECT_THAT(btree_key_exists(&btree, 4), IsFalse());

    EXPECT_THAT(btree_erase(&btree, 3), Eq(1));

    // 0
    EXPECT_THAT(btree_key_exists(&btree, 0), IsFalse());
    EXPECT_THAT(btree_key_exists(&btree, 1), IsFalse());
    EXPECT_THAT(btree_key_exists(&btree, 2), IsFalse());
    EXPECT_THAT(btree_key_exists(&btree, 3), IsFalse());
    EXPECT_THAT(btree_key_exists(&btree, 4), IsFalse());

    EXPECT_THAT(btree_erase(&btree, 0), Eq(0));
    EXPECT_THAT(btree_erase(&btree, 1), Eq(0));
    EXPECT_THAT(btree_erase(&btree, 2), Eq(0));
    EXPECT_THAT(btree_erase(&btree, 3), Eq(0));
    EXPECT_THAT(btree_erase(&btree, 4), Eq(0));

    btree_deinit(&btree);
}

TEST(NAME, reinsertion_forwards)
{
    struct btree btree;
    btree_init(&btree, 0);

    ASSERT_THAT(btree_insert_new(&btree, 0, NULL), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 1, NULL), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 2, NULL), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 3, NULL), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 4, NULL), Eq(1));

    ASSERT_THAT(btree_erase(&btree, 4), Eq(1));
    ASSERT_THAT(btree_erase(&btree, 3), Eq(1));
    ASSERT_THAT(btree_erase(&btree, 2), Eq(1));

    ASSERT_THAT(btree_insert_new(&btree, 2, NULL), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 3, NULL), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 4, NULL), Eq(1));

    EXPECT_THAT(btree_key_exists(&btree, 0), IsTrue());
    EXPECT_THAT(btree_key_exists(&btree, 1), IsTrue());
    EXPECT_THAT(btree_key_exists(&btree, 2), IsTrue());
    EXPECT_THAT(btree_key_exists(&btree, 3), IsTrue());
    EXPECT_THAT(btree_key_exists(&btree, 4), IsTrue());

    btree_deinit(&btree);
}

TEST(NAME, reinsertion_backwards)
{
    struct btree btree;
    btree_init(&btree, 0);

    ASSERT_THAT(btree_insert_new(&btree, 0, NULL), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 1, NULL), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 2, NULL), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 3, NULL), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 4, NULL), Eq(1));

    ASSERT_THAT(btree_erase(&btree, 0), Eq(1));
    ASSERT_THAT(btree_erase(&btree, 1), Eq(1));
    ASSERT_THAT(btree_erase(&btree, 2), Eq(1));

    ASSERT_THAT(btree_insert_new(&btree, 2, NULL), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 1, NULL), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 0, NULL), Eq(1));

    EXPECT_THAT(btree_key_exists(&btree, 0), IsTrue());
    EXPECT_THAT(btree_key_exists(&btree, 1), IsTrue());
    EXPECT_THAT(btree_key_exists(&btree, 2), IsTrue());
    EXPECT_THAT(btree_key_exists(&btree, 3), IsTrue());
    EXPECT_THAT(btree_key_exists(&btree, 4), IsTrue());

    btree_deinit(&btree);
}

TEST(NAME, reinsertion_random)
{
    struct btree btree;
    btree_init(&btree, 0);

    ASSERT_THAT(btree_insert_new(&btree, 26, NULL), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 44, NULL), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 82, NULL), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 41, NULL), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 70, NULL), Eq(1));

    ASSERT_THAT(btree_erase(&btree, 44), Eq(1));
    ASSERT_THAT(btree_erase(&btree, 70), Eq(1));
    ASSERT_THAT(btree_erase(&btree, 26), Eq(1));

    ASSERT_THAT(btree_insert_new(&btree, 26, NULL), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 70, NULL), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 44, NULL), Eq(1));

    EXPECT_THAT(btree_key_exists(&btree, 26), IsTrue());
    EXPECT_THAT(btree_key_exists(&btree, 44), IsTrue());
    EXPECT_THAT(btree_key_exists(&btree, 82), IsTrue());
    EXPECT_THAT(btree_key_exists(&btree, 41), IsTrue());
    EXPECT_THAT(btree_key_exists(&btree, 70), IsTrue());

    btree_deinit(&btree);
}

TEST(NAME, iterate_with_no_items)
{
    struct btree btree;
    btree_init(&btree, 0);

    int counter = 0;
    BTREE_KEYS_FOR_EACH(&btree, key)
        ++counter;
    BTREE_END_EACH
    ASSERT_THAT(counter, Eq(0));

    btree_deinit(&btree);
}


TEST(NAME, iterate_5_random_items)
{
    struct btree btree;
    btree_init(&btree, 0);

    btree_insert_new(&btree, 243, NULL);
    btree_insert_new(&btree, 256, NULL);
    btree_insert_new(&btree, 456, NULL);
    btree_insert_new(&btree, 468, NULL);
    btree_insert_new(&btree, 969, NULL);

    int counter = 0;
    BTREE_KEYS_FOR_EACH(&btree, key)
        switch(counter)
        {
            case 0 : ASSERT_THAT(key, Eq(243u)); break;
            case 1 : ASSERT_THAT(key, Eq(256u)); break;
            case 2 : ASSERT_THAT(key, Eq(456u)); break;
            case 3 : ASSERT_THAT(key, Eq(468u)); break;
            case 4 : ASSERT_THAT(key, Eq(969u)); break;
            default: ASSERT_THAT(1, Eq(0)); break;
        }
        ++counter;
    BTREE_END_EACH
    ASSERT_THAT(counter, Eq(5));

    btree_deinit(&btree);
}
