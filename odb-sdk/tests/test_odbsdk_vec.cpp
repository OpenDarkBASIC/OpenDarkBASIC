#include "gmock/gmock.h"

#define NAME vec

using namespace ::testing;

extern "C" {
#include "odb-sdk/config.h"
#include "odb-sdk/vec.h"

VEC_DECLARE_API(v16, int16_t, 16)
VEC_DEFINE_API(v16, int16_t, 16)

static void* shitty_alloc(size_t) { return NULL; }
static void* shitty_realloc(void*, size_t) { return NULL; }
#if defined(mem_alloc)
#   pragma push_macro("mem_alloc")
#   pragma push_macro("mem_realloc")
#   define mem_alloc shitty_alloc
#   define mem_realloc shitty_realloc
VEC_DECLARE_API(shitty_v16, int16_t, 16)
VEC_DEFINE_API(shitty_v16, int16_t, 16)
#   pragma pop_macro("mem_alloc")
#   pragma pop_macro("mem_realloc")
#else
#   define mem_alloc shitty_alloc
#   define mem_realloc shitty_realloc
VEC_DECLARE_API(shitty_v16, int16_t, 16)
VEC_DEFINE_API(shitty_v16, int16_t, 16)
#   undef mem_alloc
#   undef mem_realloc
#endif
}

class NAME : public Test
{
public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
        v16_free(&v16);
    }

    struct v16* v16 = nullptr;
};


TEST_F(NAME, free_null_vector_works)
{
    v16_free(&v16);
}

TEST_F(NAME, reserve_new_vector_sets_capacity)
{
    EXPECT_THAT((v16_reserve(&v16, 16)), Eq(0));
    ASSERT_THAT(v16, NotNull());
    EXPECT_THAT(v16->capacity, Eq(16));
}
TEST_F(NAME, reserve_returns_error_if_realloc_fails)
{
    EXPECT_THAT((shitty_v16_reserve((shitty_v16**)&v16, 16)), Eq(-1));
    EXPECT_THAT(v16, IsNull());
}

TEST_F(NAME, resizing_larger_than_capacity_reallocates_and_updates_size)
{
    int16_t* old_ptr = v16_emplace(&v16);
    *old_ptr = 42;
    ASSERT_THAT(v16_resize(&v16, ODBSDK_VEC_MIN_CAPACITY * 32), Eq(0));
    int16_t* new_ptr = vec_get(&v16, 0);
    EXPECT_THAT(old_ptr, Ne(new_ptr));
    EXPECT_THAT(new_ptr, Pointee(42));
    EXPECT_THAT(v16->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY * 32));
    EXPECT_THAT(vec_count(&v16), Eq(ODBSDK_VEC_MIN_CAPACITY * 32));
}
TEST_F(NAME, resizing_smaller_than_capacity_reallocates_and_updates_size)
{
    int16_t* emplaced;
    emplaced = v16_emplace(&v16);
    v16_resize(&v16, 64);

    EXPECT_THAT(v16->capacity, Eq(64u));
    EXPECT_THAT(vec_count(&v16), Eq(64u));

    v16_resize(&v16, 8);

    EXPECT_THAT(v16->capacity, Eq(8u));
    EXPECT_THAT(vec_count(&v16), Eq(8u));
}
TEST_F(NAME, resize_returns_error_if_realloc_fails)
{
    EXPECT_THAT((shitty_v16_resize((shitty_v16**)&v16, 32)), Eq(-1));
    EXPECT_THAT(v16, IsNull());
}

TEST_F(NAME, push_increments_counter)
{
    ASSERT_THAT(v16_push(&v16, 5), Eq(0));
    EXPECT_THAT(vec_count(&v16), Eq(1));
}
TEST_F(NAME, emplace_increments_counter)
{
    ASSERT_THAT((v16_emplace(&v16)), NotNull());
    EXPECT_THAT(vec_count(&v16), Eq(1));
}
TEST_F(NAME, insert_increments_counter)
{
    ASSERT_THAT((v16_insert(&v16, 0, 5)), Eq(0));
    EXPECT_THAT(vec_count(&v16), Eq(1));
}
TEST_F(NAME, insert_emplace_increments_counter)
{
    ASSERT_THAT((v16_insert_emplace(&v16, 0)), NotNull());
    EXPECT_THAT(vec_count(&v16), Eq(1));
}

TEST_F(NAME, push_sets_capacity)
{
    ASSERT_THAT((v16_insert(&v16, 0, 5)), Eq(0));
    EXPECT_THAT(v16->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY));
}
TEST_F(NAME, emplace_sets_capacity)
{
    ASSERT_THAT((v16_emplace(&v16)), NotNull());
    EXPECT_THAT(v16->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY));
}
TEST_F(NAME, insert_sets_capacity)
{
    ASSERT_THAT((v16_insert(&v16, 0, 5)), Eq(0));
    EXPECT_THAT(v16->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY));
}
TEST_F(NAME, insert_emplace_sets_capacity)
{
    ASSERT_THAT((v16_insert_emplace(&v16, 0)), NotNull());
    EXPECT_THAT(v16->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY));
}

TEST_F(NAME, push_returns_error_if_realloc_fails)
{
    EXPECT_THAT((shitty_v16_push((shitty_v16**)&v16, 5)), Eq(-1));
    EXPECT_THAT(v16, IsNull());
}
TEST_F(NAME, emplace_returns_error_if_realloc_fails)
{
    EXPECT_THAT((shitty_v16_emplace((shitty_v16**)&v16)), IsNull());
    EXPECT_THAT(v16, IsNull());
}
TEST_F(NAME, insert_returns_error_if_realloc_fails)
{
    EXPECT_THAT((shitty_v16_insert((shitty_v16**)&v16, 0, 5)), Eq(-1));
    EXPECT_THAT(v16, IsNull());
}
TEST_F(NAME, insert_emplace_returns_error_if_realloc_fails)
{
    EXPECT_THAT((shitty_v16_insert_emplace((shitty_v16**)&v16, 0)), IsNull());
    EXPECT_THAT(v16, IsNull());
}

TEST_F(NAME, push_few_values_works)
{
    EXPECT_THAT((v16_push(&v16, 5)), Eq(0));
    EXPECT_THAT((v16_push(&v16, 7)), Eq(0));
    EXPECT_THAT((v16_push(&v16, 3)), Eq(0));
    EXPECT_THAT(vec_count(&v16), Eq(3));
    EXPECT_THAT((vec_get(&v16, 0)), Pointee(5));
    EXPECT_THAT((vec_get(&v16, 1)), Pointee(7));
    EXPECT_THAT((vec_get(&v16, 2)), Pointee(3));
}
TEST_F(NAME, emplace_few_values_works)
{
    *v16_emplace(&v16) = 5;
    *v16_emplace(&v16) = 7;
    *v16_emplace(&v16) = 3;
    EXPECT_THAT(vec_count(&v16), Eq(3));
    EXPECT_THAT((vec_get(&v16, 0)), Pointee(5));
    EXPECT_THAT((vec_get(&v16, 1)), Pointee(7));
    EXPECT_THAT((vec_get(&v16, 2)), Pointee(3));
}
TEST_F(NAME, insert_few_values_works)
{
    ASSERT_THAT((v16_insert(&v16, 0, 5)), Eq(0));
    ASSERT_THAT((v16_insert(&v16, 0, 7)), Eq(0));
    ASSERT_THAT((v16_insert(&v16, 0, 3)), Eq(0));
    EXPECT_THAT(vec_count(&v16), Eq(3));
    EXPECT_THAT((vec_get(&v16, 0)), Pointee(3));
    EXPECT_THAT((vec_get(&v16, 1)), Pointee(7));
    EXPECT_THAT((vec_get(&v16, 2)), Pointee(5));
}
TEST_F(NAME, insert_emplace_few_values_works)
{
    *v16_insert_emplace(&v16, 0) = 5;
    *v16_insert_emplace(&v16, 0) = 7;
    *v16_insert_emplace(&v16, 0) = 3;
    EXPECT_THAT(vec_count(&v16), Eq(3));
    EXPECT_THAT((vec_get(&v16, 0)), Pointee(3));
    EXPECT_THAT((vec_get(&v16, 1)), Pointee(7));
    EXPECT_THAT((vec_get(&v16, 2)), Pointee(5));
}

TEST_F(NAME, push_with_expand_sets_count_and_capacity_correctly)
{
    for (int i = 0; i != ODBSDK_VEC_MIN_CAPACITY; ++i)
        ASSERT_THAT((v16_push(&v16, i)), Eq(0));

    EXPECT_THAT(vec_count(&v16), Eq(ODBSDK_VEC_MIN_CAPACITY));
    EXPECT_THAT(v16->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY));

    ASSERT_THAT((v16_push(&v16, 42)), Eq(0));
    EXPECT_THAT(vec_count(&v16), Eq(ODBSDK_VEC_MIN_CAPACITY + 1));
    EXPECT_THAT(v16->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY * ODBSDK_VEC_EXPAND_FACTOR));
}
TEST_F(NAME, emplace_with_expand_sets_count_and_capacity_correctly)
{
    for (int i = 0; i != ODBSDK_VEC_MIN_CAPACITY; ++i)
        ASSERT_THAT((v16_emplace(&v16)), NotNull());

    EXPECT_THAT(vec_count(&v16), Eq(ODBSDK_VEC_MIN_CAPACITY));
    EXPECT_THAT(v16->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY));

    ASSERT_THAT((v16_emplace(&v16)), NotNull());
    EXPECT_THAT(vec_count(&v16), Eq(ODBSDK_VEC_MIN_CAPACITY + 1));
    EXPECT_THAT(v16->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY * ODBSDK_VEC_EXPAND_FACTOR));
}
TEST_F(NAME, insert_with_expand_sets_count_and_capacity_correctly)
{
    for (int i = 0; i != ODBSDK_VEC_MIN_CAPACITY; ++i)
        ASSERT_THAT((v16_insert(&v16, 0, i)), Eq(0));

    EXPECT_THAT(vec_count(&v16), Eq(ODBSDK_VEC_MIN_CAPACITY));
    EXPECT_THAT(v16->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY));

    ASSERT_THAT((v16_insert(&v16, 3, 42)), Eq(0));
    EXPECT_THAT(vec_count(&v16), Eq(ODBSDK_VEC_MIN_CAPACITY + 1));
    EXPECT_THAT(v16->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY * ODBSDK_VEC_EXPAND_FACTOR));
}
TEST_F(NAME, insert_emplace_with_expand_sets_count_and_capacity_correctly)
{
    for (int i = 0; i != ODBSDK_VEC_MIN_CAPACITY; ++i)
        ASSERT_THAT((v16_insert_emplace(&v16, 0)), NotNull());

    EXPECT_THAT(vec_count(&v16), Eq(ODBSDK_VEC_MIN_CAPACITY));
    EXPECT_THAT(v16->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY));

    ASSERT_THAT((v16_insert_emplace(&v16, 3)), NotNull());
    EXPECT_THAT(vec_count(&v16), Eq(ODBSDK_VEC_MIN_CAPACITY + 1));
    EXPECT_THAT(v16->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY * ODBSDK_VEC_EXPAND_FACTOR));
}

TEST_F(NAME, push_with_expand_has_correct_values)
{
    for (int i = 0; i != ODBSDK_VEC_MIN_CAPACITY; ++i)
        ASSERT_THAT((v16_push(&v16, i)), Eq(0));
    ASSERT_THAT((v16_push(&v16, 42)), Eq(0));

    for (int i = 0; i != ODBSDK_VEC_MIN_CAPACITY; ++i)
        EXPECT_THAT((vec_get(&v16, i)), Pointee(i));
    EXPECT_THAT((vec_get(&v16, ODBSDK_VEC_MIN_CAPACITY)), Pointee(42));
}
TEST_F(NAME, emplace_with_expand_has_correct_values)
{
    for (int i = 0; i != ODBSDK_VEC_MIN_CAPACITY; ++i)
        *v16_emplace(&v16) = i;
    *v16_emplace(&v16) = 42;

    for (int i = 0; i != ODBSDK_VEC_MIN_CAPACITY; ++i)
        EXPECT_THAT((vec_get(&v16, i)), Pointee(i));
    EXPECT_THAT((vec_get(&v16, ODBSDK_VEC_MIN_CAPACITY)), Pointee(42));
}
TEST_F(NAME, insert_with_expand_has_correct_values)
{
    for (int i = 0; i != ODBSDK_VEC_MIN_CAPACITY; ++i)
        ASSERT_THAT((v16_insert(&v16, 0, i)), Eq(0));
    ASSERT_THAT((v16_insert(&v16, 3, 42)), Eq(0));

    for (int i = 0; i != 3; ++i)
        EXPECT_THAT((vec_get(&v16, i)), Pointee(ODBSDK_VEC_MIN_CAPACITY - i - 1));
    EXPECT_THAT((vec_get(&v16, 3)), Pointee(42));
    for (int i = 4; i != ODBSDK_VEC_MIN_CAPACITY + 1; ++i)
        EXPECT_THAT((vec_get(&v16, i)), Pointee(ODBSDK_VEC_MIN_CAPACITY - i));
}
TEST_F(NAME, insert_emplace_with_expand_has_correct_values)
{
    for (int i = 0; i != ODBSDK_VEC_MIN_CAPACITY; ++i)
        *v16_insert_emplace(&v16, 0) = i;
    *v16_insert_emplace(&v16, 3) = 42;
    
    for (int i = 0; i != 3; ++i)
        EXPECT_THAT((vec_get(&v16, i)), Pointee(ODBSDK_VEC_MIN_CAPACITY - i - 1));
    EXPECT_THAT((vec_get(&v16, 3)), Pointee(42));
    for (int i = 4; i != ODBSDK_VEC_MIN_CAPACITY + 1; ++i)
        EXPECT_THAT((vec_get(&v16, i)), Pointee(ODBSDK_VEC_MIN_CAPACITY - i));
}

TEST_F(NAME, push_expand_with_failed_realloc_returns_error)
{
    for (int i = 0; i != ODBSDK_VEC_MIN_CAPACITY; ++i)
        ASSERT_THAT((v16_push(&v16, i)), Eq(0));

    EXPECT_THAT(vec_count(&v16), Eq(ODBSDK_VEC_MIN_CAPACITY));
    EXPECT_THAT(v16->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY));

    EXPECT_THAT((shitty_v16_push((shitty_v16**)&v16, 42)), Eq(-1));
    EXPECT_THAT(vec_count(&v16), Eq(ODBSDK_VEC_MIN_CAPACITY));
    EXPECT_THAT(v16->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY));
}
TEST_F(NAME, emplace_expand_with_failed_realloc_returns_error)
{
    for (int i = 0; i != ODBSDK_VEC_MIN_CAPACITY; ++i)
        ASSERT_THAT((v16_emplace(&v16)), NotNull());

    EXPECT_THAT(vec_count(&v16), Eq(ODBSDK_VEC_MIN_CAPACITY));
    EXPECT_THAT(v16->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY));

    EXPECT_THAT((shitty_v16_emplace((shitty_v16**)&v16)), IsNull());
    EXPECT_THAT(vec_count(&v16), Eq(ODBSDK_VEC_MIN_CAPACITY));
    EXPECT_THAT(v16->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY));
}
TEST_F(NAME, insert_expand_with_failed_realloc_returns_error)
{
    for (int i = 0; i != ODBSDK_VEC_MIN_CAPACITY; ++i)
        ASSERT_THAT((v16_insert(&v16, 0, i)), Eq(0));

    EXPECT_THAT(vec_count(&v16), Eq(ODBSDK_VEC_MIN_CAPACITY));
    EXPECT_THAT(v16->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY));

    EXPECT_THAT((shitty_v16_insert((shitty_v16**)&v16, 3, 42)), Eq(-1));
    EXPECT_THAT(vec_count(&v16), Eq(ODBSDK_VEC_MIN_CAPACITY));
    EXPECT_THAT(v16->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY));
}
TEST_F(NAME, insert_emplace_expand_with_failed_realloc_returns_error)
{
    for (int i = 0; i != ODBSDK_VEC_MIN_CAPACITY; ++i)
        ASSERT_THAT((v16_insert_emplace(&v16, 0)), NotNull());

    EXPECT_THAT(vec_count(&v16), Eq(ODBSDK_VEC_MIN_CAPACITY));
    EXPECT_THAT(v16->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY));

    EXPECT_THAT((shitty_v16_insert_emplace((shitty_v16**)&v16, 3)), IsNull());
    EXPECT_THAT(vec_count(&v16), Eq(ODBSDK_VEC_MIN_CAPACITY));
    EXPECT_THAT(v16->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY));
}

TEST_F(NAME, inserting_preserves_existing_elements)
{
    v16_push(&v16, 53);
    v16_push(&v16, 24);
    v16_push(&v16, 73);
    v16_push(&v16, 43);
    v16_push(&v16, 65);

    v16_insert(&v16, 2, 68); // middle insertion
    EXPECT_THAT((vec_get(&v16, 0)), Pointee(53));
    EXPECT_THAT((vec_get(&v16, 1)), Pointee(24));
    EXPECT_THAT((vec_get(&v16, 2)), Pointee(68));
    EXPECT_THAT((vec_get(&v16, 3)), Pointee(73));
    EXPECT_THAT((vec_get(&v16, 4)), Pointee(43));
    EXPECT_THAT((vec_get(&v16, 5)), Pointee(65));

    v16_insert(&v16, 0, 16); // beginning insertion
    EXPECT_THAT((vec_get(&v16, 0)), Pointee(16));
    EXPECT_THAT((vec_get(&v16, 1)), Pointee(53));
    EXPECT_THAT((vec_get(&v16, 2)), Pointee(24));
    EXPECT_THAT((vec_get(&v16, 3)), Pointee(68));
    EXPECT_THAT((vec_get(&v16, 4)), Pointee(73));
    EXPECT_THAT((vec_get(&v16, 5)), Pointee(43));
    EXPECT_THAT((vec_get(&v16, 6)), Pointee(65));

    v16_insert(&v16, 7, 82); // end insertion
    EXPECT_THAT((vec_get(&v16, 0)), Pointee(16));
    EXPECT_THAT((vec_get(&v16, 1)), Pointee(53));
    EXPECT_THAT((vec_get(&v16, 2)), Pointee(24));
    EXPECT_THAT((vec_get(&v16, 3)), Pointee(68));
    EXPECT_THAT((vec_get(&v16, 4)), Pointee(73));
    EXPECT_THAT((vec_get(&v16, 5)), Pointee(43));
    EXPECT_THAT((vec_get(&v16, 6)), Pointee(65));
    EXPECT_THAT((vec_get(&v16, 7)), Pointee(82));

    v16_insert(&v16, 7, 37); // end insertion
    EXPECT_THAT((vec_get(&v16, 0)), Pointee(16));
    EXPECT_THAT((vec_get(&v16, 1)), Pointee(53));
    EXPECT_THAT((vec_get(&v16, 2)), Pointee(24));
    EXPECT_THAT((vec_get(&v16, 3)), Pointee(68));
    EXPECT_THAT((vec_get(&v16, 4)), Pointee(73));
    EXPECT_THAT((vec_get(&v16, 5)), Pointee(43));
    EXPECT_THAT((vec_get(&v16, 6)), Pointee(65));
    EXPECT_THAT((vec_get(&v16, 7)), Pointee(37));
    EXPECT_THAT((vec_get(&v16, 8)), Pointee(82));
}

TEST_F(NAME, insert_emplacing_preserves_existing_elements)
{
    v16_push(&v16, 53);
    v16_push(&v16, 24);
    v16_push(&v16, 73);
    v16_push(&v16, 43);
    v16_push(&v16, 65);

    *v16_insert_emplace(&v16, 2) = 68; // middle insertion
    EXPECT_THAT((vec_get(&v16, 0)), Pointee(53));
    EXPECT_THAT((vec_get(&v16, 1)), Pointee(24));
    EXPECT_THAT((vec_get(&v16, 2)), Pointee(68));
    EXPECT_THAT((vec_get(&v16, 3)), Pointee(73));
    EXPECT_THAT((vec_get(&v16, 4)), Pointee(43));
    EXPECT_THAT((vec_get(&v16, 5)), Pointee(65));

    *v16_insert_emplace(&v16, 0) = 16; // beginning insertion
    EXPECT_THAT((vec_get(&v16, 0)), Pointee(16));
    EXPECT_THAT((vec_get(&v16, 1)), Pointee(53));
    EXPECT_THAT((vec_get(&v16, 2)), Pointee(24));
    EXPECT_THAT((vec_get(&v16, 3)), Pointee(68));
    EXPECT_THAT((vec_get(&v16, 4)), Pointee(73));
    EXPECT_THAT((vec_get(&v16, 5)), Pointee(43));
    EXPECT_THAT((vec_get(&v16, 6)), Pointee(65));

    *v16_insert_emplace(&v16, 7) = 82; // end insertion
    EXPECT_THAT((vec_get(&v16, 0)), Pointee(16));
    EXPECT_THAT((vec_get(&v16, 1)), Pointee(53));
    EXPECT_THAT((vec_get(&v16, 2)), Pointee(24));
    EXPECT_THAT((vec_get(&v16, 3)), Pointee(68));
    EXPECT_THAT((vec_get(&v16, 4)), Pointee(73));
    EXPECT_THAT((vec_get(&v16, 5)), Pointee(43));
    EXPECT_THAT((vec_get(&v16, 6)), Pointee(65));
    EXPECT_THAT((vec_get(&v16, 7)), Pointee(82));

    *v16_insert_emplace(&v16, 7) = 37; // end insertion
    EXPECT_THAT((vec_get(&v16, 0)), Pointee(16));
    EXPECT_THAT((vec_get(&v16, 1)), Pointee(53));
    EXPECT_THAT((vec_get(&v16, 2)), Pointee(24));
    EXPECT_THAT((vec_get(&v16, 3)), Pointee(68));
    EXPECT_THAT((vec_get(&v16, 4)), Pointee(73));
    EXPECT_THAT((vec_get(&v16, 5)), Pointee(43));
    EXPECT_THAT((vec_get(&v16, 6)), Pointee(65));
    EXPECT_THAT((vec_get(&v16, 7)), Pointee(37));
    EXPECT_THAT((vec_get(&v16, 8)), Pointee(82));
}

TEST_F(NAME, clear_null_vector_works)
{
    v16_clear(&v16);
}

TEST_F(NAME, clear_keeps_buffer_and_resets_count)
{
    for (int i = 0; i != ODBSDK_VEC_MIN_CAPACITY*2; ++i)
        ASSERT_THAT((v16_push(&v16, i)), Eq(0));

    EXPECT_THAT(vec_count(&v16), Eq(ODBSDK_VEC_MIN_CAPACITY*2));
    EXPECT_THAT(v16->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY*2));
    v16_clear(&v16);
    EXPECT_THAT(vec_count(&v16), Eq(0u));
    EXPECT_THAT(v16->capacity, Eq(ODBSDK_VEC_MIN_CAPACITY*2));
}

TEST_F(NAME, compact_null_vector_works)
{
    v16_compact(&v16);
}

TEST_F(NAME, compact_sets_capacity)
{
    v16_push(&v16, 9);
    v16_compact(&v16);
    ASSERT_THAT(v16, NotNull());
    EXPECT_THAT(vec_count(&v16), Eq(1));
    EXPECT_THAT(v16->capacity, Eq(1));
}

TEST_F(NAME, compact_removes_excess_space)
{
    v16_push(&v16, 1);
    v16_push(&v16, 2);
    v16_push(&v16, 3);
    v16_pop(&v16);
    v16_pop(&v16);
    v16_compact(&v16);
    ASSERT_THAT(v16, NotNull());
    EXPECT_THAT(vec_count(&v16), Eq(1));
    EXPECT_THAT(v16->capacity, Eq(1));
}

TEST_F(NAME, clear_and_compact_deletes_buffer)
{
    v16_push(&v16, 9);
    v16_clear(&v16);
    v16_compact(&v16);
    EXPECT_THAT(v16, IsNull());
}

TEST_F(NAME, clear_compact_null_vector_works)
{
    v16_clear_compact(&v16);
}

TEST_F(NAME, clear_compact_deletes_buffer)
{
    v16_push(&v16, 9);
    v16_clear_compact(&v16);
    EXPECT_THAT(v16, IsNull());
}

TEST_F(NAME, pop_returns_pushed_values)
{
    v16_push(&v16, 3);
    v16_push(&v16, 2);
    v16_push(&v16, 6);
    EXPECT_THAT(v16_pop(&v16), Pointee(6));
    v16_push(&v16, 23);
    v16_push(&v16, 21);
    EXPECT_THAT(v16_pop(&v16), Pointee(21));
    EXPECT_THAT(v16_pop(&v16), Pointee(23));
    EXPECT_THAT(v16_pop(&v16), Pointee(2));
    EXPECT_THAT(v16_pop(&v16), Pointee(3));

    ASSERT_THAT(v16, NotNull());
    EXPECT_THAT(vec_count(&v16), Eq(0u));
}

TEST_F(NAME, pop_returns_emplaced_values)
{
    *v16_emplace(&v16) = 53;
    *v16_emplace(&v16) = 24;
    *v16_emplace(&v16) = 73;
    EXPECT_THAT(v16_pop(&v16), Pointee(73));
    EXPECT_THAT(vec_count(&v16), Eq(2));
    *v16_emplace(&v16) = 28;
    *v16_emplace(&v16) = 72;
    EXPECT_THAT(v16_pop(&v16), Pointee(72));
    EXPECT_THAT(v16_pop(&v16), Pointee(28));
    EXPECT_THAT(v16_pop(&v16), Pointee(24));
    EXPECT_THAT(v16_pop(&v16), Pointee(53));

    EXPECT_THAT(vec_count(&v16), Eq(0u));
    EXPECT_THAT(v16, NotNull());
}

TEST_F(NAME, popping_preserves_existing_elements)
{
    v16_push(&v16, 53);
    v16_push(&v16, 24);
    v16_push(&v16, 73);
    v16_push(&v16, 43);
    v16_push(&v16, 24);

    v16_pop(&v16);
    EXPECT_THAT((vec_get(&v16, 1)), Pointee(24));
    EXPECT_THAT((vec_get(&v16, 3)), Pointee(43));
    EXPECT_THAT((vec_get(&v16, 2)), Pointee(73));
    EXPECT_THAT((vec_get(&v16, 0)), Pointee(53));
}

TEST_F(NAME, get_last_element)
{
    v16_push(&v16, 53);
    EXPECT_THAT((vec_last(&v16)), Pointee(53));
    v16_push(&v16, 24);
    EXPECT_THAT((vec_last(&v16)), Pointee(24));
    v16_push(&v16, 73);
    EXPECT_THAT((vec_last(&v16)), Pointee(73));
}

TEST_F(NAME, get_first_element)
{
    v16_push(&v16, 53);
    EXPECT_THAT((vec_first(&v16)), Pointee(53));
    v16_push(&v16, 24);
    EXPECT_THAT((vec_first(&v16)), Pointee(53));
    v16_push(&v16, 73);
    EXPECT_THAT((vec_first(&v16)), Pointee(53));
}

TEST_F(NAME, get_element_random_access)
{
    v16_push(&v16, 53);
    v16_push(&v16, 24);
    v16_push(&v16, 73);
    v16_push(&v16, 43);
    EXPECT_THAT(((vec_get(&v16, 1))), Pointee(24));
    EXPECT_THAT(((vec_get(&v16, 3))), Pointee(43));
    EXPECT_THAT(((vec_get(&v16, 2))), Pointee(73));
    EXPECT_THAT(((vec_get(&v16, 0))), Pointee(53));
}

TEST_F(NAME, rget_element_random_access)
{
    v16_push(&v16, 53);
    v16_push(&v16, 24);
    v16_push(&v16, 73);
    v16_push(&v16, 43);
    EXPECT_THAT((vec_rget(&v16, 1)), Pointee(73));
    EXPECT_THAT((vec_rget(&v16, 3)), Pointee(53));
    EXPECT_THAT((vec_rget(&v16, 2)), Pointee(24));
    EXPECT_THAT((vec_rget(&v16, 0)), Pointee(43));
}

TEST_F(NAME, erasing_by_index_preserves_existing_elements)
{
    v16_push(&v16, 53);
    v16_push(&v16, 24);
    v16_push(&v16, 73);
    v16_push(&v16, 43);
    v16_push(&v16, 65);

    v16_erase(&v16, 1);
    EXPECT_THAT((vec_get(&v16, 0)), Pointee(53));
    EXPECT_THAT((vec_get(&v16, 1)), Pointee(73));
    EXPECT_THAT((vec_get(&v16, 2)), Pointee(43));
    EXPECT_THAT((vec_get(&v16, 3)), Pointee(65));

    v16_erase(&v16, 1);
    EXPECT_THAT((vec_get(&v16, 0)), Pointee(53));
    EXPECT_THAT((vec_get(&v16, 1)), Pointee(43));
    EXPECT_THAT((vec_get(&v16, 2)), Pointee(65));
}

TEST_F(NAME, for_each_zero_elements)
{
    int16_t* value;
    int counter = 0;
    vec_for_each(&v16, value)
        counter++;

    EXPECT_THAT(counter, Eq(0));
}

TEST_F(NAME, for_each_one_element)
{
    v16_push(&v16, 1);

    int counter = 0;
    int16_t* value;
    vec_for_each(&v16, value)
    {
        counter++;
        EXPECT_THAT(value, Pointee(Eq(1)));
    }

    EXPECT_THAT(counter, Eq(1));
}

TEST_F(NAME, for_each_three_elements)
{
    v16_push(&v16, 1);
    v16_push(&v16, 2);
    v16_push(&v16, 3);

    int counter = 0;
    int16_t* value;
    vec_for_each(&v16, value)
    {
        counter++;
        EXPECT_THAT(value, Pointee(Eq(counter)));
    }

    EXPECT_THAT(counter, Eq(3));
}

