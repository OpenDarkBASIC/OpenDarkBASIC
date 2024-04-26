#include "gmock/gmock.h"
#include "vh/btree.h"

#define NAME vh_btree

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

    btree_init(&btree, sizeof(data_t));
    EXPECT_THAT(btree.count, Eq(0u));
    EXPECT_THAT(btree.capacity, Eq(0u));
    EXPECT_THAT(btree.count, Eq(0u));
    EXPECT_THAT(btree.data, IsNull());
    EXPECT_THAT(btree.value_size, Eq(sizeof(data_t)));
}

TEST(NAME, create_initializes_btree)
{
    struct btree* btree = btree_alloc(sizeof(data_t));
    ASSERT_THAT(btree, NotNull());
    EXPECT_THAT(btree_capacity(btree), Eq(0u));
    EXPECT_THAT(btree_count(btree), Eq(0u));
    EXPECT_THAT(btree->data, IsNull());
    EXPECT_THAT(btree->value_size, Eq(sizeof(data_t)));
    btree_free(btree);
}

TEST(NAME, insertion_forwards)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int a=56, b=45, c=18, d=27, e=84;
    ASSERT_THAT(btree_insert_new(&btree, 0, &a), Eq(1));  ASSERT_THAT(btree_count(&btree), Eq(1u));
    ASSERT_THAT(btree_insert_new(&btree, 1, &b), Eq(1));  ASSERT_THAT(btree_count(&btree), Eq(2u));
    ASSERT_THAT(btree_insert_new(&btree, 2, &c), Eq(1));  ASSERT_THAT(btree_count(&btree), Eq(3u));
    ASSERT_THAT(btree_insert_new(&btree, 3, &d), Eq(1));  ASSERT_THAT(btree_count(&btree), Eq(4u));
    ASSERT_THAT(btree_insert_new(&btree, 4, &e), Eq(1));  ASSERT_THAT(btree_count(&btree), Eq(5u));

    EXPECT_THAT(*(int*)btree_find(&btree, 0), Eq(a));
    EXPECT_THAT(*(int*)btree_find(&btree, 1), Eq(b));
    EXPECT_THAT(*(int*)btree_find(&btree, 2), Eq(c));
    EXPECT_THAT(*(int*)btree_find(&btree, 3), Eq(d));
    EXPECT_THAT(*(int*)btree_find(&btree, 4), Eq(e));
    EXPECT_THAT(btree_find(&btree, 5), IsNull());

    btree_deinit(&btree);
}

TEST(NAME, insertion_backwards)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int a=56, b=45, c=18, d=27, e=84;
    ASSERT_THAT(btree_insert_new(&btree, 4, &a), Eq(1));  ASSERT_THAT(btree_count(&btree), Eq(1u));
    ASSERT_THAT(btree_insert_new(&btree, 3, &b), Eq(1));  ASSERT_THAT(btree_count(&btree), Eq(2u));
    ASSERT_THAT(btree_insert_new(&btree, 2, &c), Eq(1));  ASSERT_THAT(btree_count(&btree), Eq(3u));
    ASSERT_THAT(btree_insert_new(&btree, 1, &d), Eq(1));  ASSERT_THAT(btree_count(&btree), Eq(4u));
    ASSERT_THAT(btree_insert_new(&btree, 0, &e), Eq(1));  ASSERT_THAT(btree_count(&btree), Eq(5u));

    EXPECT_THAT(*(int*)btree_find(&btree, 0), Eq(e));
    EXPECT_THAT(*(int*)btree_find(&btree, 1), Eq(d));
    EXPECT_THAT(*(int*)btree_find(&btree, 2), Eq(c));
    EXPECT_THAT(*(int*)btree_find(&btree, 3), Eq(b));
    EXPECT_THAT(*(int*)btree_find(&btree, 4), Eq(a));
    EXPECT_THAT(btree_find(&btree, 5), IsNull());

    btree_deinit(&btree);
}

TEST(NAME, insertion_random)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int a=56, b=45, c=18, d=27, e=84;
    ASSERT_THAT(btree_insert_new(&btree, 26, &a), Eq(1));  ASSERT_THAT(btree_count(&btree), Eq(1u));
    ASSERT_THAT(btree_insert_new(&btree, 44, &b), Eq(1));  ASSERT_THAT(btree_count(&btree), Eq(2u));
    ASSERT_THAT(btree_insert_new(&btree, 82, &c), Eq(1));  ASSERT_THAT(btree_count(&btree), Eq(3u));
    ASSERT_THAT(btree_insert_new(&btree, 41, &d), Eq(1));  ASSERT_THAT(btree_count(&btree), Eq(4u));
    ASSERT_THAT(btree_insert_new(&btree, 70, &e), Eq(1));  ASSERT_THAT(btree_count(&btree), Eq(5u));

    EXPECT_THAT(*(int*)btree_find(&btree, 26), Eq(a));
    EXPECT_THAT(*(int*)btree_find(&btree, 41), Eq(d));
    EXPECT_THAT(*(int*)btree_find(&btree, 44), Eq(b));
    EXPECT_THAT(*(int*)btree_find(&btree, 70), Eq(e));
    EXPECT_THAT(*(int*)btree_find(&btree, 82), Eq(c));

    btree_deinit(&btree);
}

TEST(NAME, insert_new_with_realloc_shifts_data_correctly)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    btree_size midway = VH_BTREE_MIN_CAPACITY / 2;

    int value = 0x55;
    btree_reserve(&btree, VH_BTREE_MIN_CAPACITY);
    for (int i = 0; i != VH_BTREE_MIN_CAPACITY; ++i)
    {
        if (i < midway)
            ASSERT_THAT(btree_insert_new(&btree, i, &value), Eq(1));
        else
            ASSERT_THAT(btree_insert_new(&btree, i+1, &value), Eq(1));
    }

    // Make sure we didn't cause a realloc yet
    ASSERT_THAT(btree_capacity(&btree), Eq(VH_BTREE_MIN_CAPACITY));
    ASSERT_THAT(btree_insert_new(&btree, midway, &value), Eq(1));
    // Now it should have reallocated
    ASSERT_THAT(btree_capacity(&btree), Gt(VH_BTREE_MIN_CAPACITY));

    // Check all values are there
    for (int i = 0; i != VH_BTREE_MIN_CAPACITY+1; ++i)
        EXPECT_THAT(btree_find(&btree, i), NotNull()) << "i: " << i << ", midway: " << midway;

    btree_deinit(&btree);
}

TEST(NAME, clear_keeps_underlying_buffer)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int a = 53;
    btree_insert_new(&btree, 0, &a);
    btree_insert_new(&btree, 1, &a);
    btree_insert_new(&btree, 2, &a);

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
    btree_init(&btree, sizeof(int));
    btree_compact(&btree);
    btree_deinit(&btree);
}

TEST(NAME, compact_reduces_capacity_and_keeps_elements_in_tact)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int a = 53;
    for (int i = 0; i != VH_BTREE_MIN_CAPACITY * 3; ++i)
        ASSERT_THAT(btree_insert_new(&btree, i, &a), Eq(1));
    for (int i = 0; i != VH_BTREE_MIN_CAPACITY; ++i)
        btree_erase(&btree, i);

    btree_size old_capacity = btree_capacity(&btree);
    btree_compact(&btree);
    EXPECT_THAT(btree_capacity(&btree), Lt(old_capacity));
    EXPECT_THAT(btree_count(&btree), Eq(VH_BTREE_MIN_CAPACITY * 2));
    EXPECT_THAT(btree_capacity(&btree), Eq(VH_BTREE_MIN_CAPACITY * 2));
    EXPECT_THAT(btree.data, NotNull());

    btree_deinit(&btree);
}

TEST(NAME, clear_and_compact_deletes_underlying_buffer)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int a=53;
    btree_insert_new(&btree, 0, &a);
    btree_insert_new(&btree, 1, &a);
    btree_insert_new(&btree, 2, &a);

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
    btree_init(&btree, sizeof(int));

    int a=53;
    EXPECT_THAT(btree_insert_new(&btree, 0, &a), Eq(1));
    EXPECT_THAT(btree_insert_new(&btree, 0, &a), Eq(0));

    btree_deinit(&btree);
}
TEST(NAME, set_existing_fails_if_key_doesnt_exist)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int a=53;
    EXPECT_THAT(btree_set_existing(&btree, 0, &a), Eq(0));

    btree_deinit(&btree);
}

TEST(NAME, set_existing_key_works_on_keys_that_exist)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int a=53, b=77, c=99, d=12;
    EXPECT_THAT(btree_insert_new(&btree, 7, &a), Eq(1));
    EXPECT_THAT(btree_insert_new(&btree, 3, &b), Eq(1));
    EXPECT_THAT(btree_set_existing(&btree, 3, &d), Eq(1));
    EXPECT_THAT(btree_set_existing(&btree, 7, &c), Eq(1));
    EXPECT_THAT(*static_cast<int*>(btree_find(&btree, 3)), Eq(12));
    EXPECT_THAT(*static_cast<int*>(btree_find(&btree, 7)), Eq(99));

    btree_deinit(&btree);
}

TEST(NAME, insert_or_get_works)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int a=53, b=77, *get;
    EXPECT_THAT(btree_insert_or_get(&btree, 0, &a, (void**)&get), Eq(1));
    EXPECT_THAT(get, AllOf(NotNull(), Pointee(Eq(53))));
    EXPECT_THAT(static_cast<int*>(btree_find(&btree, 0)), AllOf(NotNull(), Pointee(Eq(53))));
    EXPECT_THAT(btree_count(&btree), Eq(1));

    EXPECT_THAT(btree_insert_or_get(&btree, 1, &b, (void**)&get), Eq(1));
    EXPECT_THAT(get, AllOf(NotNull(), Pointee(Eq(77))));
    EXPECT_THAT(static_cast<int*>(btree_find(&btree, 1)), AllOf(NotNull(), Pointee(Eq(77))));
    EXPECT_THAT(btree_count(&btree), Eq(2));

    EXPECT_THAT(btree_insert_or_get(&btree, 0, &a, (void**)&get), Eq(0));
    EXPECT_THAT(get, AllOf(NotNull(), Pointee(Eq(53))));
    EXPECT_THAT(btree_count(&btree), Eq(2));

    EXPECT_THAT(btree_insert_or_get(&btree, 1, &b, (void**)&get), Eq(0));
    EXPECT_THAT(get, AllOf(NotNull(), Pointee(Eq(77))));
    EXPECT_THAT(btree_count(&btree), Eq(2));

    btree_deinit(&btree);
}

TEST(NAME, insert_or_get_with_realloc_shifts_data_correctly)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    btree_size midway = VH_BTREE_MIN_CAPACITY / 2;

    int value = 0x55, *get;
    btree_reserve(&btree, VH_BTREE_MIN_CAPACITY);
    for (int i = 0; i != VH_BTREE_MIN_CAPACITY; ++i)
    {
        if (i < midway)
            ASSERT_THAT(btree_insert_or_get(&btree, i, &value, (void**)&get), Eq(1));
        else
            ASSERT_THAT(btree_insert_or_get(&btree, i+1, &value, (void**)&get), Eq(1));
    }

    // Make sure we didn't cause a realloc yet
    get = nullptr;
    ASSERT_THAT(btree_capacity(&btree), Eq(VH_BTREE_MIN_CAPACITY));
    ASSERT_THAT(btree_insert_or_get(&btree, midway, &value, (void**)&get), Eq(1));
    // Now it should have reallocated
    ASSERT_THAT(btree_capacity(&btree), Gt(VH_BTREE_MIN_CAPACITY));

    // Check all values are there
    for (int i = 0; i != VH_BTREE_MIN_CAPACITY+1; ++i)
    {
        get = nullptr;
        EXPECT_THAT(btree_insert_or_get(&btree, i, &value, (void**)&get), Eq(0)) << "i: " << i << ", midway: " << midway;
        EXPECT_THAT(get, AllOf(NotNull(), Pointee(Eq(0x55))));
    }

    btree_deinit(&btree);
}

TEST(NAME, find_on_empty_btree_doesnt_crash)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    EXPECT_THAT(btree_find(&btree, 3), IsNull());

    btree_deinit(&btree);
}

TEST(NAME, find_key_on_empty_btree_doesnt_crash)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int value=55;
    EXPECT_THAT(btree_find_key(&btree, &value), IsNull());

    btree_deinit(&btree);
}

TEST(NAME, find_and_compare_on_empty_btree_doesnt_crash)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int value=55;
    EXPECT_THAT(btree_find_and_compare(&btree, 34, &value), IsFalse());

    btree_deinit(&btree);
}

TEST(NAME, search_for_key_using_existing_value_works)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int a=53, b=77, c=99;
    btree_insert_new(&btree, 3, &a);
    btree_insert_new(&btree, 8, &b);
    btree_insert_new(&btree, 2, &c);

    EXPECT_THAT(btree_find_key(&btree, &a), AllOf(NotNull(), Pointee(Eq(3))));
    EXPECT_THAT(btree_find_key(&btree, &b), AllOf(NotNull(), Pointee(Eq(8))));
    EXPECT_THAT(btree_find_key(&btree, &c), AllOf(NotNull(), Pointee(Eq(2))));

    btree_deinit(&btree);
}

TEST(NAME, search_for_key_using_non_existing_value_returns_null)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int a=53, b=77, c=99;
    btree_insert_new(&btree, 3, &a);
    btree_insert_new(&btree, 8, &b);
    btree_insert_new(&btree, 2, &c);

    int d = 12;
    EXPECT_THAT(btree_find_key(&btree, &d), IsNull());

    btree_deinit(&btree);
}

TEST(NAME, find_and_compare_non_existing_key_returns_false)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int a=53, b=77, c=99;
    btree_insert_new(&btree, 3, &a);
    btree_insert_new(&btree, 8, &b);
    btree_insert_new(&btree, 2, &c);

    int d=53;
    EXPECT_THAT(btree_find_and_compare(&btree, 4, &d), IsFalse());

    btree_deinit(&btree);
}

TEST(NAME, find_and_compare_existing_key_and_different_value_returns_false)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int a=53, b=77, c=99;
    btree_insert_new(&btree, 3, &a);
    btree_insert_new(&btree, 8, &b);
    btree_insert_new(&btree, 2, &c);

    int d=53;
    EXPECT_THAT(btree_find_and_compare(&btree, 8, &d), IsFalse());

    btree_deinit(&btree);
}

TEST(NAME, find_and_compare_existing_key_and_matching_value_returns_true)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int a=53, b=77, c=99;
    btree_insert_new(&btree, 3, &a);
    btree_insert_new(&btree, 8, &b);
    btree_insert_new(&btree, 2, &c);

    int d=53;
    EXPECT_THAT(btree_find_and_compare(&btree, 3, &d), IsTrue());

    btree_deinit(&btree);
}

TEST(NAME, key_exists_returns_false_if_key_doesnt_exist)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int a=53, b=77, c=99;
    btree_insert_new(&btree, 3, &a);
    btree_insert_new(&btree, 8, &b);
    btree_insert_new(&btree, 2, &c);

    int d=53;
    EXPECT_THAT(btree_key_exists(&btree, 4), IsFalse());

    btree_deinit(&btree);
}

TEST(NAME, key_exists_returns_true_if_key_does_exist)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int a=53, b=77, c=99;
    btree_insert_new(&btree, 3, &a);
    btree_insert_new(&btree, 8, &b);
    btree_insert_new(&btree, 2, &c);

    int d=53;
    EXPECT_THAT(btree_key_exists(&btree, 3), IsTrue());

    btree_deinit(&btree);
}

TEST(NAME, erase_by_key)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int a=56, b=45, c=18, d=27, e=84;
    btree_insert_new(&btree, 0, &a);
    btree_insert_new(&btree, 1, &b);
    btree_insert_new(&btree, 2, &c);
    btree_insert_new(&btree, 3, &d);
    btree_insert_new(&btree, 4, &e);

    EXPECT_THAT(btree_erase(&btree, 2), Eq(1));

    // 4
    EXPECT_THAT((int*)btree_find(&btree, 0), AllOf(NotNull(), Pointee(Eq(a))));
    EXPECT_THAT((int*)btree_find(&btree, 1), AllOf(NotNull(), Pointee(Eq(b))));
    EXPECT_THAT((int*)btree_find(&btree, 2), IsNull());
    EXPECT_THAT((int*)btree_find(&btree, 3), AllOf(NotNull(), Pointee(Eq(d))));
    EXPECT_THAT((int*)btree_find(&btree, 4), AllOf(NotNull(), Pointee(Eq(e))));

    EXPECT_THAT(btree_erase(&btree, 4), Eq(1));

    // 3
    EXPECT_THAT((int*)btree_find(&btree, 0), AllOf(NotNull(), Pointee(Eq(a))));
    EXPECT_THAT((int*)btree_find(&btree, 1), AllOf(NotNull(), Pointee(Eq(b))));
    EXPECT_THAT((int*)btree_find(&btree, 2), IsNull());
    EXPECT_THAT((int*)btree_find(&btree, 3), AllOf(NotNull(), Pointee(Eq(d))));
    EXPECT_THAT((int*)btree_find(&btree, 4), IsNull());

    EXPECT_THAT(btree_erase(&btree, 0), Eq(1));

    // 2
    EXPECT_THAT((int*)btree_find(&btree, 0), IsNull());
    EXPECT_THAT((int*)btree_find(&btree, 1), AllOf(NotNull(), Pointee(Eq(b))));
    EXPECT_THAT((int*)btree_find(&btree, 2), IsNull());
    EXPECT_THAT((int*)btree_find(&btree, 3), AllOf(NotNull(), Pointee(Eq(d))));
    EXPECT_THAT((int*)btree_find(&btree, 4), IsNull());

    EXPECT_THAT(btree_erase(&btree, 1), Eq(1));

    // 1
    EXPECT_THAT((int*)btree_find(&btree, 0), IsNull());
    EXPECT_THAT((int*)btree_find(&btree, 1), IsNull());
    EXPECT_THAT((int*)btree_find(&btree, 2), IsNull());
    EXPECT_THAT((int*)btree_find(&btree, 3), AllOf(NotNull(), Pointee(Eq(d))));
    EXPECT_THAT((int*)btree_find(&btree, 4), IsNull());

    EXPECT_THAT(btree_erase(&btree, 3), Eq(1));

    // 0
    EXPECT_THAT((int*)btree_find(&btree, 0), IsNull());
    EXPECT_THAT((int*)btree_find(&btree, 1), IsNull());
    EXPECT_THAT((int*)btree_find(&btree, 2), IsNull());
    EXPECT_THAT((int*)btree_find(&btree, 3), IsNull());
    EXPECT_THAT((int*)btree_find(&btree, 4), IsNull());

    EXPECT_THAT(btree_erase(&btree, 0), Eq(0));
    EXPECT_THAT(btree_erase(&btree, 1), Eq(0));
    EXPECT_THAT(btree_erase(&btree, 2), Eq(0));
    EXPECT_THAT(btree_erase(&btree, 3), Eq(0));
    EXPECT_THAT(btree_erase(&btree, 4), Eq(0));

    btree_deinit(&btree);
}

TEST(NAME, erase_by_value_on_empty_btree_doesnt_crash)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int a=56;
    EXPECT_THAT(btree_erase_value(&btree, &a), Eq(BTREE_INVALID_KEY));

    btree_deinit(&btree);
}

TEST(NAME, erase_by_value)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int a=56, b=45, c=18, d=27, e=84;
    btree_insert_new(&btree, 0, &a);
    btree_insert_new(&btree, 1, &b);
    btree_insert_new(&btree, 2, &c);
    btree_insert_new(&btree, 3, &d);
    btree_insert_new(&btree, 4, &e);

    EXPECT_THAT(btree_erase_value(&btree, &c), Eq(2));

    // 4
    EXPECT_THAT((int*)btree_find(&btree, 0), AllOf(NotNull(), Pointee(Eq(a))));
    EXPECT_THAT((int*)btree_find(&btree, 1), AllOf(NotNull(), Pointee(Eq(b))));
    EXPECT_THAT((int*)btree_find(&btree, 2), IsNull());
    EXPECT_THAT((int*)btree_find(&btree, 3), AllOf(NotNull(), Pointee(Eq(d))));
    EXPECT_THAT((int*)btree_find(&btree, 4), AllOf(NotNull(), Pointee(Eq(e))));

    EXPECT_THAT(btree_erase_value(&btree, &e), Eq(4));

    // 3
    EXPECT_THAT((int*)btree_find(&btree, 0), AllOf(NotNull(), Pointee(Eq(a))));
    EXPECT_THAT((int*)btree_find(&btree, 1), AllOf(NotNull(), Pointee(Eq(b))));
    EXPECT_THAT((int*)btree_find(&btree, 2), IsNull());
    EXPECT_THAT((int*)btree_find(&btree, 3), AllOf(NotNull(), Pointee(Eq(d))));
    EXPECT_THAT((int*)btree_find(&btree, 4), IsNull());

    EXPECT_THAT(btree_erase_value(&btree, &a), Eq(0));

    // 2
    EXPECT_THAT((int*)btree_find(&btree, 0), IsNull());
    EXPECT_THAT((int*)btree_find(&btree, 1), AllOf(NotNull(), Pointee(Eq(b))));
    EXPECT_THAT((int*)btree_find(&btree, 2), IsNull());
    EXPECT_THAT((int*)btree_find(&btree, 3), AllOf(NotNull(), Pointee(Eq(d))));
    EXPECT_THAT((int*)btree_find(&btree, 4), IsNull());

    EXPECT_THAT(btree_erase_value(&btree, &b), Eq(1));

    // 1
    EXPECT_THAT((int*)btree_find(&btree, 0), IsNull());
    EXPECT_THAT((int*)btree_find(&btree, 1), IsNull());
    EXPECT_THAT((int*)btree_find(&btree, 2), IsNull());
    EXPECT_THAT((int*)btree_find(&btree, 3), AllOf(NotNull(), Pointee(Eq(d))));
    EXPECT_THAT((int*)btree_find(&btree, 4), IsNull());

    EXPECT_THAT(btree_erase_value(&btree, &d), Eq(3));

    // 0
    EXPECT_THAT((int*)btree_find(&btree, 0), IsNull());
    EXPECT_THAT((int*)btree_find(&btree, 1), IsNull());
    EXPECT_THAT((int*)btree_find(&btree, 2), IsNull());
    EXPECT_THAT((int*)btree_find(&btree, 3), IsNull());
    EXPECT_THAT((int*)btree_find(&btree, 4), IsNull());

    EXPECT_THAT(btree_erase_value(&btree, &a), Eq(BTREE_INVALID_KEY));
    EXPECT_THAT(btree_erase_value(&btree, &b), Eq(BTREE_INVALID_KEY));
    EXPECT_THAT(btree_erase_value(&btree, &c), Eq(BTREE_INVALID_KEY));
    EXPECT_THAT(btree_erase_value(&btree, &d), Eq(BTREE_INVALID_KEY));
    EXPECT_THAT(btree_erase_value(&btree, &e), Eq(BTREE_INVALID_KEY));

    btree_deinit(&btree);
}

TEST(NAME, reinsertion_forwards)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int a=56, b=45, c=18, d=27, e=84;
    ASSERT_THAT(btree_insert_new(&btree, 0, &a), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 1, &b), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 2, &c), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 3, &d), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 4, &e), Eq(1));

    ASSERT_THAT(btree_erase(&btree, 4), Eq(1));
    ASSERT_THAT(btree_erase(&btree, 3), Eq(1));
    ASSERT_THAT(btree_erase(&btree, 2), Eq(1));

    ASSERT_THAT(btree_insert_new(&btree, 2, &c), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 3, &d), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 4, &e), Eq(1));

    EXPECT_THAT((int*)btree_find(&btree, 0), AllOf(NotNull(), Pointee(Eq(a))));
    EXPECT_THAT((int*)btree_find(&btree, 1), AllOf(NotNull(), Pointee(Eq(b))));
    EXPECT_THAT((int*)btree_find(&btree, 2), AllOf(NotNull(), Pointee(Eq(c))));
    EXPECT_THAT((int*)btree_find(&btree, 3), AllOf(NotNull(), Pointee(Eq(d))));
    EXPECT_THAT((int*)btree_find(&btree, 4), AllOf(NotNull(), Pointee(Eq(e))));

    btree_deinit(&btree);
}

TEST(NAME, reinsertion_backwards)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int a=56, b=45, c=18, d=27, e=84;
    ASSERT_THAT(btree_insert_new(&btree, 0, &a), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 1, &b), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 2, &c), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 3, &d), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 4, &e), Eq(1));

    ASSERT_THAT(btree_erase(&btree, 0), Eq(1));
    ASSERT_THAT(btree_erase(&btree, 1), Eq(1));
    ASSERT_THAT(btree_erase(&btree, 2), Eq(1));

    ASSERT_THAT(btree_insert_new(&btree, 2, &c), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 1, &b), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 0, &a), Eq(1));

    EXPECT_THAT((int*)btree_find(&btree, 0), AllOf(NotNull(), Pointee(Eq(a))));
    EXPECT_THAT((int*)btree_find(&btree, 1), AllOf(NotNull(), Pointee(Eq(b))));
    EXPECT_THAT((int*)btree_find(&btree, 2), AllOf(NotNull(), Pointee(Eq(c))));
    EXPECT_THAT((int*)btree_find(&btree, 3), AllOf(NotNull(), Pointee(Eq(d))));
    EXPECT_THAT((int*)btree_find(&btree, 4), AllOf(NotNull(), Pointee(Eq(e))));

    btree_deinit(&btree);
}

TEST(NAME, reinsertion_random)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int a=56, b=45, c=18, d=27, e=84;
    ASSERT_THAT(btree_insert_new(&btree, 26, &a), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 44, &b), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 82, &c), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 41, &d), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 70, &e), Eq(1));

    ASSERT_THAT(btree_erase(&btree, 44), Eq(1));
    ASSERT_THAT(btree_erase(&btree, 70), Eq(1));
    ASSERT_THAT(btree_erase(&btree, 26), Eq(1));

    ASSERT_THAT(btree_insert_new(&btree, 26, &a), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 70, &e), Eq(1));
    ASSERT_THAT(btree_insert_new(&btree, 44, &b), Eq(1));

    EXPECT_THAT((int*)btree_find(&btree, 26), AllOf(NotNull(), Pointee(Eq(a))));
    EXPECT_THAT((int*)btree_find(&btree, 44), AllOf(NotNull(), Pointee(Eq(b))));
    EXPECT_THAT((int*)btree_find(&btree, 82), AllOf(NotNull(), Pointee(Eq(c))));
    EXPECT_THAT((int*)btree_find(&btree, 41), AllOf(NotNull(), Pointee(Eq(d))));
    EXPECT_THAT((int*)btree_find(&btree, 70), AllOf(NotNull(), Pointee(Eq(e))));

    btree_deinit(&btree);
}

TEST(NAME, get_any_value)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int a = 6;
    EXPECT_THAT(btree_get_any_value(&btree), IsNull());
    btree_insert_new(&btree, 45, &a);
    EXPECT_THAT((int*)btree_get_any_value(&btree), AllOf(NotNull(), Pointee(Eq(6))));
    btree_erase(&btree, 45);
    EXPECT_THAT(btree_get_any_value(&btree), IsNull());

    btree_deinit(&btree);
}


TEST(NAME, iterate_with_no_items)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int counter = 0;
    BTREE_FOR_EACH(&btree, int, key, value)
        ++counter;
    BTREE_END_EACH
    ASSERT_THAT(counter, Eq(0));

    btree_deinit(&btree);
}


TEST(NAME, iterate_5_random_items)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int a=79579, b=235, c=347, d=124, e=457;
    btree_insert_new(&btree, 243, &a);
    btree_insert_new(&btree, 256, &b);
    btree_insert_new(&btree, 456, &c);
    btree_insert_new(&btree, 468, &d);
    btree_insert_new(&btree, 969, &e);

    int counter = 0;
    BTREE_FOR_EACH(&btree, int, key, value)
        switch(counter)
        {
            case 0 : ASSERT_THAT(key, Eq(243u)); ASSERT_THAT(a, Eq(*value)); break;
            case 1 : ASSERT_THAT(key, Eq(256u)); ASSERT_THAT(b, Eq(*value)); break;
            case 2 : ASSERT_THAT(key, Eq(456u)); ASSERT_THAT(c, Eq(*value)); break;
            case 3 : ASSERT_THAT(key, Eq(468u)); ASSERT_THAT(d, Eq(*value)); break;
            case 4 : ASSERT_THAT(key, Eq(969u)); ASSERT_THAT(e, Eq(*value)); break;
            default: ASSERT_THAT(1, Eq(0)); break;
        }
        ++counter;
    BTREE_END_EACH
    ASSERT_THAT(counter, Eq(5));

    btree_deinit(&btree);
}

TEST(NAME, erase_in_for_loop)
{
    struct btree btree;
    btree_init(&btree, sizeof(int));

    int a=79579, b=235, c=347, d=124, e=457;
    btree_insert_new(&btree, 243, &a);
    btree_insert_new(&btree, 256, &b);
    btree_insert_new(&btree, 456, &c);
    btree_insert_new(&btree, 468, &d);
    btree_insert_new(&btree, 969, &e);

    int counter = 0;
    BTREE_FOR_EACH(&btree, int, key, value)
        if(key == 256u)
            BTREE_ERASE_CURRENT_ITEM_IN_FOR_LOOP(&btree, key);
        ++counter;
    BTREE_END_EACH
    EXPECT_THAT(counter, Eq(5u));

    EXPECT_THAT((int*)btree_find(&btree, 243), AllOf(NotNull(), Pointee(Eq(a))));
    EXPECT_THAT((int*)btree_find(&btree, 256), IsNull());
    EXPECT_THAT((int*)btree_find(&btree, 456), AllOf(NotNull(), Pointee(Eq(c))));
    EXPECT_THAT((int*)btree_find(&btree, 468), AllOf(NotNull(), Pointee(Eq(d))));
    EXPECT_THAT((int*)btree_find(&btree, 969), AllOf(NotNull(), Pointee(Eq(e))));

    counter = 0;
    BTREE_FOR_EACH(&btree, int, key, value)
        if(key == 969)
            BTREE_ERASE_CURRENT_ITEM_IN_FOR_LOOP(&btree, key);
        ++counter;
    BTREE_END_EACH
    EXPECT_THAT(counter, Eq(4u));

    EXPECT_THAT((int*)btree_find(&btree, 243), AllOf(NotNull(), Pointee(Eq(a))));
    EXPECT_THAT((int*)btree_find(&btree, 256), IsNull());
    EXPECT_THAT((int*)btree_find(&btree, 456), AllOf(NotNull(), Pointee(Eq(c))));
    EXPECT_THAT((int*)btree_find(&btree, 468), AllOf(NotNull(), Pointee(Eq(d))));
    EXPECT_THAT((int*)btree_find(&btree, 969), IsNull());

    counter = 0;
    BTREE_FOR_EACH(&btree, int, key, value)
        if(key == 243)
            BTREE_ERASE_CURRENT_ITEM_IN_FOR_LOOP(&btree, key);
        ++counter;
    BTREE_END_EACH
    EXPECT_THAT(counter, Eq(3u));

    EXPECT_THAT((int*)btree_find(&btree, 243), IsNull());
    EXPECT_THAT((int*)btree_find(&btree, 256), IsNull());
    EXPECT_THAT((int*)btree_find(&btree, 456), AllOf(NotNull(), Pointee(Eq(c))));
    EXPECT_THAT((int*)btree_find(&btree, 468), AllOf(NotNull(), Pointee(Eq(d))));
    EXPECT_THAT((int*)btree_find(&btree, 969), IsNull());

    btree_deinit(&btree);
}
