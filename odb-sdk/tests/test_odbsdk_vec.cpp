#include "odb-sdk/config.h"
extern "C" {
#include "odb-sdk/vec.h"
}

#include "gmock/gmock.h"

#define NAME odbsdk_vec

using namespace ::testing;

class NAME : public Test
{
public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
        vec_free(vec);
    }

    VEC(int16_t,int16_t)* vec = nullptr;
};

static int shitty_vec_realloc(void**, size_t) { return -1; }

TEST_F(NAME, free_null_vector_works)
{
    vec_free(NULL);
}

TEST_F(NAME, reserve_new_vector_sets_capacity)
{
    EXPECT_THAT((vec_reserve(vec, 16)), Eq(0));
    ASSERT_THAT(vec, NotNull());
    EXPECT_THAT(vec->capacity, Eq(16));
}

TEST_F(NAME, reserve_returns_error_if_realloc_fails)
{
#define _vec_realloc shitty_vec_realloc
    EXPECT_THAT((vec_reserve(vec, 16)), Eq(-1));
#undef _vec_realloc
    EXPECT_THAT(vec, IsNull());
}

TEST_F(NAME, push_few_values_works)
{
    EXPECT_THAT((vec_push(vec, 5)), Eq(0));
    EXPECT_THAT((vec_push(vec, 7)), Eq(0));
    EXPECT_THAT((vec_push(vec, 3)), Eq(0));
    EXPECT_THAT(vec->count, Eq(3));
    EXPECT_THAT(vec->data[0], Eq(5));
    EXPECT_THAT(vec->data[1], Eq(7));
    EXPECT_THAT(vec->data[2], Eq(3));
}

TEST_F(NAME, push_increments_count_and_causes_realloc_by_factor)
{
    for(int i = 0; i != ODBSDK_VEC_MIN_CAPACITY; ++i)
        ASSERT_THAT((vec_push(vec, i)), Eq(0));

    EXPECT_THAT(vec->count, Eq(ODBSDK_VEC_MIN_CAPACITY));
    EXPECT_THAT(vec->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY));

    ASSERT_THAT((vec_push(vec, 42)), Eq(0));
    EXPECT_THAT(vec->count, Eq(ODBSDK_VEC_MIN_CAPACITY + 1));
    EXPECT_THAT(vec->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY * ODBSDK_VEC_EXPAND_FACTOR));
}

TEST_F(NAME, emplace_increments_count_and_causes_realloc_by_factor)
{
    for(int i = 0; i != ODBSDK_VEC_MIN_CAPACITY; ++i)
        ASSERT_THAT((vec_emplace(vec)), NotNull());

    EXPECT_THAT(vec->count, Eq(ODBSDK_VEC_MIN_CAPACITY));
    EXPECT_THAT(vec->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY));

    vec_emplace(vec);
    EXPECT_THAT(vec->count, Eq(ODBSDK_VEC_MIN_CAPACITY + 1));
    EXPECT_THAT(vec->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY * ODBSDK_VEC_EXPAND_FACTOR));
}

TEST_F(NAME, clear_keeps_buffer_and_resets_count)
{
    for(int i = 0; i != ODBSDK_VEC_MIN_CAPACITY*2; ++i)
        ASSERT_THAT((vec_push(vec, i)), Eq(0));

    EXPECT_THAT(vec->count, Eq(ODBSDK_VEC_MIN_CAPACITY*2));
    EXPECT_THAT(vec->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY*2));
    vec_clear(vec);
    EXPECT_THAT(vec->count, Eq(0u));
    EXPECT_THAT(vec->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY*2));
}

TEST_F(NAME, compact_sets_capacity)
{
    vec_push(vec, 9);
    vec_compact(vec);
    ASSERT_THAT(vec, NotNull());
    EXPECT_THAT(vec->count, Eq(1));
    EXPECT_THAT(vec->capacity, Eq(1));
}

TEST_F(NAME, clear_and_compact_deletes_buffer)
{
    vec_push(vec, 9);
    vec_clear(vec);
    vec_compact(vec);
    EXPECT_THAT(vec, IsNull());
}

TEST_F(NAME, clear_compact_deletes_buffer)
{
    vec_push(vec, 9);
    vec_clear_compact(vec);
    EXPECT_THAT(vec, IsNull());
}

TEST_F(NAME, pop_returns_pushed_values)
{
    vec_push(vec, 3);
    vec_push(vec, 2);
    vec_push(vec, 6);
    EXPECT_THAT((int16_t*)vec_pop(vec), Pointee(6));
    vec_push(vec, 23);
    vec_push(vec, 21);
    EXPECT_THAT((int16_t*)vec_pop(vec), Pointee(21));
    EXPECT_THAT((int16_t*)vec_pop(vec), Pointee(23));
    EXPECT_THAT((int16_t*)vec_pop(vec), Pointee(2));
    EXPECT_THAT((int16_t*)vec_pop(vec), Pointee(3));

    ASSERT_THAT(vec, NotNull());
    EXPECT_THAT(vec->count, Eq(0u));
}

TEST_F(NAME, pop_returns_emplaced_values)
{
    int16_t* emplaced;

    emplaced = vec_emplace(vec); *emplaced = 53;
    emplaced = vec_emplace(vec); *emplaced = 24;
    emplaced = vec_emplace(vec); *emplaced = 73;
    EXPECT_THAT(vec_pop(vec), Pointee(73));
    emplaced = vec_emplace(vec); *emplaced = 28;
    emplaced = vec_emplace(vec); *emplaced = 72;
    EXPECT_THAT(vec_pop(vec), Pointee(72));
    EXPECT_THAT(vec_pop(vec), Pointee(28));
    EXPECT_THAT(vec_pop(vec), Pointee(24));
    EXPECT_THAT(vec_pop(vec), Pointee(53));

    EXPECT_THAT(vec->count, Eq(0u));
    EXPECT_THAT(vec, NotNull());
}

TEST_F(NAME, pop_empty_vector)
{
    int16_t* emplaced;
    emplaced = vec_emplace(vec); *emplaced = 21;
    vec_pop(vec);
    EXPECT_THAT(vec_pop(vec), IsNull());
    EXPECT_THAT(vec_pop(vec), IsNull());
    EXPECT_THAT(vec_pop(vec), IsNull());
    EXPECT_THAT(vec->count, Eq(0u));
    EXPECT_THAT(vec, NotNull());
}

TEST_F(NAME, pop_clear_freed_vector)
{
    EXPECT_THAT(vec_pop(vec), IsNull());
    EXPECT_THAT(vec_pop(vec), IsNull());
    EXPECT_THAT(vec_pop(vec), IsNull());
    EXPECT_THAT(vec, IsNull());
}

TEST_F(NAME, popping_preserves_existing_elements)
{
    vec_push(vec, 53);
    vec_push(vec, 24);
    vec_push(vec, 73);
    vec_push(vec, 43);
    vec_push(vec, 24);

    vec_pop(vec);
    EXPECT_THAT(vec->data[1], Eq(24));
    EXPECT_THAT(vec->data[3], Eq(43));
    EXPECT_THAT(vec->data[2], Eq(73));
    EXPECT_THAT(vec->data[0], Eq(53));
}

TEST_F(NAME, get_element_random_access)
{
    vec_push(vec, 53);
    vec_push(vec, 24);
    vec_push(vec, 73);
    vec_push(vec, 43);
    EXPECT_THAT(vec->data[1], Eq(24));
    EXPECT_THAT(vec->data[3], Eq(43));
    EXPECT_THAT(vec->data[2], Eq(73));
    EXPECT_THAT(vec->data[0], Eq(53));
}

//TEST_F(NAME, erasing_by_index_preserves_existing_elements)
//{
//    vec_push(vec, 53);
//    vec_push(vec, 24);
//    vec_push(vec, 73);
//    vec_push(vec, 43);
//    vec_push(vec, 65);
//
//    vec_erase_index(&vec, 1);
//    EXPECT_THAT(*(int*)vec_get(&vec, 0), Eq(53));
//    EXPECT_THAT(*(int*)vec_get(&vec, 1), Eq(73));
//    EXPECT_THAT(*(int*)vec_get(&vec, 2), Eq(43));
//    EXPECT_THAT(*(int*)vec_get(&vec, 3), Eq(65));
//
//    vec_erase_index(&vec, 1);
//    EXPECT_THAT(*(int*)vec_get(&vec, 0), Eq(53));
//    EXPECT_THAT(*(int*)vec_get(&vec, 1), Eq(43));
//    EXPECT_THAT(*(int*)vec_get(&vec, 2), Eq(65));
//}
//
//TEST_F(NAME, erasing_by_element_preserves_existing_elements)
//{
//    int* emplaced;
//    emplaced = (int*)vec_emplace(&vec); *emplaced = 53;
//    emplaced = (int*)vec_emplace(&vec); *emplaced = 24;
//    emplaced = (int*)vec_emplace(&vec); *emplaced = 73;
//    emplaced = (int*)vec_emplace(&vec); *emplaced = 43;
//    emplaced = (int*)vec_emplace(&vec); *emplaced = 65;
//
//    vec_erase_element(&vec, vec_get(&vec, 1));
//    EXPECT_THAT(*(int*)vec_get(&vec, 0), Eq(53));
//    EXPECT_THAT(*(int*)vec_get(&vec, 1), Eq(73));
//    EXPECT_THAT(*(int*)vec_get(&vec, 2), Eq(43));
//    EXPECT_THAT(*(int*)vec_get(&vec, 3), Eq(65));
//
//    vec_erase_element(&vec, vec_get(&vec, 1));
//    EXPECT_THAT(*(int*)vec_get(&vec, 0), Eq(53));
//    EXPECT_THAT(*(int*)vec_get(&vec, 1), Eq(43));
//    EXPECT_THAT(*(int*)vec_get(&vec, 2), Eq(65));
//}
//
//TEST_F(NAME, inserting_preserves_existing_elements)
//{
//    int* emplaced;
//    emplaced = (int*)vec_emplace(&vec); *emplaced = 53;
//    emplaced = (int*)vec_emplace(&vec); *emplaced = 24;
//    emplaced = (int*)vec_emplace(&vec); *emplaced = 73;
//    emplaced = (int*)vec_emplace(&vec); *emplaced = 43;
//    emplaced = (int*)vec_emplace(&vec); *emplaced = 65;
//
//    int x = 68;
//    vec_insert(&vec, 2, &x); // middle insertion
//
//    EXPECT_THAT(*(int*)vec_get(&vec, 0), Eq(53));
//    EXPECT_THAT(*(int*)vec_get(&vec, 1), Eq(24));
//    EXPECT_THAT(*(int*)vec_get(&vec, 2), Eq(68));
//    EXPECT_THAT(*(int*)vec_get(&vec, 3), Eq(73));
//    EXPECT_THAT(*(int*)vec_get(&vec, 4), Eq(43));
//    EXPECT_THAT(*(int*)vec_get(&vec, 5), Eq(65));
//
//    x = 16;
//    vec_insert(&vec, 0, &x); // beginning insertion
//
//    EXPECT_THAT(*(int*)vec_get(&vec, 0), Eq(16));
//    EXPECT_THAT(*(int*)vec_get(&vec, 1), Eq(53));
//    EXPECT_THAT(*(int*)vec_get(&vec, 2), Eq(24));
//    EXPECT_THAT(*(int*)vec_get(&vec, 3), Eq(68));
//    EXPECT_THAT(*(int*)vec_get(&vec, 4), Eq(73));
//    EXPECT_THAT(*(int*)vec_get(&vec, 5), Eq(43));
//    EXPECT_THAT(*(int*)vec_get(&vec, 6), Eq(65));
//
//    x = 82;
//    vec_insert(&vec, 7, &x); // end insertion
//
//    EXPECT_THAT(*(int*)vec_get(&vec, 0), Eq(16));
//    EXPECT_THAT(*(int*)vec_get(&vec, 1), Eq(53));
//    EXPECT_THAT(*(int*)vec_get(&vec, 2), Eq(24));
//    EXPECT_THAT(*(int*)vec_get(&vec, 3), Eq(68));
//    EXPECT_THAT(*(int*)vec_get(&vec, 4), Eq(73));
//    EXPECT_THAT(*(int*)vec_get(&vec, 5), Eq(43));
//    EXPECT_THAT(*(int*)vec_get(&vec, 6), Eq(65));
//    EXPECT_THAT(*(int*)vec_get(&vec, 7), Eq(82));
//
//    x = 37;
//    vec_insert(&vec, 7, &x); // end insertion
//
//    EXPECT_THAT(*(int*)vec_get(&vec, 0), Eq(16));
//    EXPECT_THAT(*(int*)vec_get(&vec, 1), Eq(53));
//    EXPECT_THAT(*(int*)vec_get(&vec, 2), Eq(24));
//    EXPECT_THAT(*(int*)vec_get(&vec, 3), Eq(68));
//    EXPECT_THAT(*(int*)vec_get(&vec, 4), Eq(73));
//    EXPECT_THAT(*(int*)vec_get(&vec, 5), Eq(43));
//    EXPECT_THAT(*(int*)vec_get(&vec, 6), Eq(65));
//    EXPECT_THAT(*(int*)vec_get(&vec, 7), Eq(37));
//    EXPECT_THAT(*(int*)vec_get(&vec, 8), Eq(82));
//}
//
//TEST_F(NAME, insert_emplacing_preserves_existing_elements)
//{
//    int* emplaced;
//    emplaced = (int*)vec_emplace(&vec); *emplaced = 53;
//    emplaced = (int*)vec_emplace(&vec); *emplaced = 24;
//    emplaced = (int*)vec_emplace(&vec); *emplaced = 73;
//    emplaced = (int*)vec_emplace(&vec); *emplaced = 43;
//    emplaced = (int*)vec_emplace(&vec); *emplaced = 65;
//
//    emplaced = (int*)vec_insert_emplace(&vec, 2); *emplaced = 68; // middle insertion
//
//    EXPECT_THAT(*(int*)vec_get(&vec, 0), Eq(53));
//    EXPECT_THAT(*(int*)vec_get(&vec, 1), Eq(24));
//    EXPECT_THAT(*(int*)vec_get(&vec, 2), Eq(68));
//    EXPECT_THAT(*(int*)vec_get(&vec, 3), Eq(73));
//    EXPECT_THAT(*(int*)vec_get(&vec, 4), Eq(43));
//    EXPECT_THAT(*(int*)vec_get(&vec, 5), Eq(65));
//
//    emplaced = (int*)vec_insert_emplace(&vec, 0); *emplaced = 16; // beginning insertion
//
//    EXPECT_THAT(*(int*)vec_get(&vec, 0), Eq(16));
//    EXPECT_THAT(*(int*)vec_get(&vec, 1), Eq(53));
//    EXPECT_THAT(*(int*)vec_get(&vec, 2), Eq(24));
//    EXPECT_THAT(*(int*)vec_get(&vec, 3), Eq(68));
//    EXPECT_THAT(*(int*)vec_get(&vec, 4), Eq(73));
//    EXPECT_THAT(*(int*)vec_get(&vec, 5), Eq(43));
//    EXPECT_THAT(*(int*)vec_get(&vec, 6), Eq(65));
//
//    emplaced = (int*)vec_insert_emplace(&vec, 7); *emplaced = 82; // end insertion
//
//    EXPECT_THAT(*(int*)vec_get(&vec, 0), Eq(16));
//    EXPECT_THAT(*(int*)vec_get(&vec, 1), Eq(53));
//    EXPECT_THAT(*(int*)vec_get(&vec, 2), Eq(24));
//    EXPECT_THAT(*(int*)vec_get(&vec, 3), Eq(68));
//    EXPECT_THAT(*(int*)vec_get(&vec, 4), Eq(73));
//    EXPECT_THAT(*(int*)vec_get(&vec, 5), Eq(43));
//    EXPECT_THAT(*(int*)vec_get(&vec, 6), Eq(65));
//    EXPECT_THAT(*(int*)vec_get(&vec, 7), Eq(82));
//
//    emplaced = (int*)vec_insert_emplace(&vec, 7); *emplaced = 37; // end insertion
//
//    EXPECT_THAT(*(int*)vec_get(&vec, 0), Eq(16));
//    EXPECT_THAT(*(int*)vec_get(&vec, 1), Eq(53));
//    EXPECT_THAT(*(int*)vec_get(&vec, 2), Eq(24));
//    EXPECT_THAT(*(int*)vec_get(&vec, 3), Eq(68));
//    EXPECT_THAT(*(int*)vec_get(&vec, 4), Eq(73));
//    EXPECT_THAT(*(int*)vec_get(&vec, 5), Eq(43));
//    EXPECT_THAT(*(int*)vec_get(&vec, 6), Eq(65));
//    EXPECT_THAT(*(int*)vec_get(&vec, 7), Eq(37));
//    EXPECT_THAT(*(int*)vec_get(&vec, 8), Eq(82));
//}
//
//TEST_F(NAME, resizing_larger_than_capacity_reallocates_and_updates_size)
//{
//    int* old_ptr;
//
//    old_ptr = (int*)vec_emplace(&vec);
//    *old_ptr = 42;
//    vec_resize(&vec, ODBSDK_VEC_MIN_CAPACITY * 1024 * 1024);
//    int* new_ptr = (int*)vec_get(&vec, 0);
//    EXPECT_THAT(old_ptr, Ne(new_ptr));
//    EXPECT_THAT(*new_ptr, Eq(42));
//    EXPECT_THAT(vec->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY * 1024 * 1024));
//    EXPECT_THAT(vec->count, Eq(ODBSDK_VEC_MIN_CAPACITY * 1024 * 1024));
//}
//
//TEST_F(NAME, resizing_smaller_than_capacity_updates_size_but_not_capacity)
//{
//    int* emplaced;
//    emplaced = (int*)vec_emplace(&vec);
//    vec_resize(&vec, 64);
//
//    EXPECT_THAT(vec->capacity, Eq(64u));
//    EXPECT_THAT(vec->count, Eq(64u));
//
//    vec_resize(&vec, 8);
//
//    EXPECT_THAT(vec->capacity, Eq(64u));
//    EXPECT_THAT(vec->count, Eq(8u));
//}
//
//TEST_F(NAME, for_each_zero_elements)
//{
//    int counter = 0;
//    VEC_FOR_EACH(&vec, int, value)
//        counter++;
//    VEC_END_EACH
//
//    EXPECT_THAT(counter, Eq(0));
//}
//
//TEST_F(NAME, for_each_one_element)
//{
//    *(int*)vec_emplace(&vec) = 1;
//
//    int counter = 0;
//    VEC_FOR_EACH(&vec, int, value)
//        counter++;
//        EXPECT_THAT(value, Pointee(Eq(1)));
//    VEC_END_EACH
//
//    EXPECT_THAT(counter, Eq(1));
//}
//
//TEST_F(NAME, for_each_three_elements)
//{
//    *(int*)vec_emplace(&vec) = 1;
//    *(int*)vec_emplace(&vec) = 2;
//    *(int*)vec_emplace(&vec) = 3;
//
//    int counter = 0;
//    VEC_FOR_EACH(&vec, int, value)
//        counter++;
//        EXPECT_THAT(value, Pointee(Eq(counter)));
//    VEC_END_EACH
//
//    EXPECT_THAT(counter, Eq(3));
//}
//
//TEST_F(NAME, for_each_r_zero_elements)
//{
//    int counter = 0;
//    VEC_FOR_EACH_R(&vec, int, value)
//        counter++;
//    VEC_END_EACH
//
//    EXPECT_THAT(counter, Eq(0));
//}
//
//TEST_F(NAME, for_each_r_one_element)
//{
//    *(int*)vec_emplace(&vec) = 1;
//
//    int counter = 0;
//    VEC_FOR_EACH_R(&vec, int, value)
//        counter++;
//        EXPECT_THAT(value, Pointee(Eq(1)));
//    VEC_END_EACH
//
//    EXPECT_THAT(counter, Eq(1));
//}
//
//TEST_F(NAME, for_each_r_three_elements)
//{
//    *(int*)vec_emplace(&vec) = 1;
//    *(int*)vec_emplace(&vec) = 2;
//    *(int*)vec_emplace(&vec) = 3;
//
//    int counter = 0;
//    VEC_FOR_EACH_R(&vec, int, value)
//        EXPECT_THAT(value, Pointee(Eq(3-counter)));
//        counter++;
//    VEC_END_EACH
//
//    EXPECT_THAT(counter, Eq(3));
//}
//
//TEST_F(NAME, for_each_range_zero_elements)
//{
//    int counter = 0;
//    VEC_FOR_EACH_RANGE(&vec, int, value, 0, 0)
//        counter++;
//    VEC_END_EACH
//
//    EXPECT_THAT(counter, Eq(0));
//}
//
//TEST_F(NAME, for_each_range_one_element)
//{
//    *(int*)vec_emplace(&vec) = 1;
//
//    int counter = 0;
//    VEC_FOR_EACH_RANGE(&vec, int, value, 0, 1)
//        counter++;
//        EXPECT_THAT(value, Pointee(Eq(1)));
//    VEC_END_EACH
//    EXPECT_THAT(counter, Eq(1));
//
//    counter = 0;
//    VEC_FOR_EACH_RANGE(&vec, int, value, 0, 0)
//        counter++;
//    VEC_END_EACH
//    EXPECT_THAT(counter, Eq(0));
//
//    counter = 0;
//    VEC_FOR_EACH_RANGE(&vec, int, value, 1, 1)
//        counter++;
//    VEC_END_EACH
//    EXPECT_THAT(counter, Eq(0));
//}
//
//TEST_F(NAME, for_each_range_five_elements)
//{
//    *(int*)vec_emplace(&vec) = 1;
//    *(int*)vec_emplace(&vec) = 2;
//    *(int*)vec_emplace(&vec) = 3;
//    *(int*)vec_emplace(&vec) = 4;
//    *(int*)vec_emplace(&vec) = 5;
//
//    int counter = 0;
//    VEC_FOR_EACH_RANGE(&vec, int, value, 0, 5)
//        counter++;
//        EXPECT_THAT(value, Pointee(Eq(counter)));
//    VEC_END_EACH
//    EXPECT_THAT(counter, Eq(5));
//
//    counter = 0;
//    VEC_FOR_EACH_RANGE(&vec, int, value, 0, 3)
//        counter++;
//        EXPECT_THAT(value, Pointee(Eq(counter)));
//    VEC_END_EACH
//    EXPECT_THAT(counter, Eq(3));
//
//    counter = 0;
//    VEC_FOR_EACH_RANGE(&vec, int, value, 2, 5)
//        counter++;
//        EXPECT_THAT(value, Pointee(Eq(counter+2)));
//    VEC_END_EACH
//    EXPECT_THAT(counter, Eq(3));
//}
//
//TEST_F(NAME, for_each_range_r_zero_elements)
//{
//    int counter = 0;
//    VEC_FOR_EACH_RANGE_R(&vec, int, value, 0, 0)
//        counter++;
//    VEC_END_EACH
//
//    EXPECT_THAT(counter, Eq(0));
//}
//
//TEST_F(NAME, for_each_range_r_one_element)
//{
//    *(int*)vec_emplace(&vec) = 1;
//
//    int counter = 0;
//    VEC_FOR_EACH_RANGE_R(&vec, int, value, 0, 1)
//        counter++;
//        EXPECT_THAT(value, Pointee(Eq(1)));
//    VEC_END_EACH
//    EXPECT_THAT(counter, Eq(1));
//
//    counter = 0;
//    VEC_FOR_EACH_RANGE_R(&vec, int, value, 0, 0)
//        counter++;
//    VEC_END_EACH
//    EXPECT_THAT(counter, Eq(0));
//
//    counter = 0;
//    VEC_FOR_EACH_RANGE_R(&vec, int, value, 1, 1)
//        counter++;
//    VEC_END_EACH
//    EXPECT_THAT(counter, Eq(0));
//}
//
//TEST_F(NAME, for_each_range_r_five_elements)
//{
//    *(int*)vec_emplace(&vec) = 1;
//    *(int*)vec_emplace(&vec) = 2;
//    *(int*)vec_emplace(&vec) = 3;
//    *(int*)vec_emplace(&vec) = 4;
//    *(int*)vec_emplace(&vec) = 5;
//
//    int counter = 0;
//    VEC_FOR_EACH_RANGE_R(&vec, int, value, 0, 5)
//        counter++;
//        EXPECT_THAT(value, Pointee(Eq(6-counter)));
//    VEC_END_EACH
//    EXPECT_THAT(counter, Eq(5));
//
//    counter = 0;
//    VEC_FOR_EACH_RANGE_R(&vec, int, value, 0, 3)
//        counter++;
//        EXPECT_THAT(value, Pointee(Eq(4-counter)));
//    VEC_END_EACH
//    EXPECT_THAT(counter, Eq(3));
//
//    counter = 0;
//    VEC_FOR_EACH_RANGE_R(&vec, int, value, 2, 5)
//        counter++;
//        EXPECT_THAT(value, Pointee(Eq(6-counter)));
//    VEC_END_EACH
//    EXPECT_THAT(counter, Eq(3));
//}
