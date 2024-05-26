#include "gmock/gmock.h"

extern "C" {
#include "odb-sdk/mem.h"
#include "odb-sdk/rb.h"

struct obj
{
    uint64_t a, b, c, d;
};

bool
operator==(const struct obj& a, const struct obj& b)
{
    return a.a == b.a && a.b == b.b && a.c == b.c && a.d == b.d;
}

RB_DECLARE_API(rbobj, struct obj, 16)
// RB_DEFINE_API(rbobj, struct obj, 16)
void
rbobj_deinit(struct rbobj* rb)
{
    if (rb->mem)
        mem_free(rb->mem);
}

int
rbobj_resize(struct rbobj* rb, int16_t elems)
{
    void*    new_mem;
    mem_size bytes = sizeof(*rb->mem) + sizeof(rb->mem->data[0]) * (elems - 1);

    ODBSDK_DEBUG_ASSERT(IS_POWER_OF_2(elems));
    new_mem = mem_realloc(rb->mem, bytes);
    if (new_mem == NULL)
        return log_oom(bytes, "rb_resize()");
    if (rb->mem == NULL)
    {
        *(void**)&rb->mem = new_mem;
        rb->mem->read = 0;
        rb->mem->write = 0;
    }
    else
    {
        *(void**)&rb->mem = new_mem;
    }

    /* Is the data wrapped? */
    if (rb->mem->read > rb->mem->write)
    {
        memmove(
            rb->mem->data + rb->mem->capacity,
            rb->mem->data,
            rb->mem->write * sizeof(rb->mem->data[0]));
        rb->mem->write += rb->mem->capacity;
    }

    rb->mem->capacity = elems;
    return 0;
}
}

#define NAME odbsdk_rb

using namespace testing;

struct NAME : public Test
{
    void
    SetUp() override
    {
        rbobj_init(&rbobj);
        rbobj_resize(&rbobj, 32), Eq(0);
    }

    void
    TearDown() override
    {
        rbobj_deinit(&rbobj);
    }

    struct rbobj rbobj;
};

TEST_F(NAME, resize_initializes_members)
{
    rbobj_deinit(&rbobj);
    rbobj_init(&rbobj);
    ASSERT_THAT(rbobj_resize(&rbobj, 32), Eq(0));
    ASSERT_THAT(rbobj.mem, NotNull());
    ASSERT_THAT(rbobj.mem->capacity, Eq(32));
    ASSERT_THAT(rbobj.mem->read, Eq(0));
    ASSERT_THAT(rbobj.mem->write, Eq(0));
}

TEST_F(NAME, space_left)
{
    rbobj.mem->read = 0;
    rbobj.mem->write = 0;
    EXPECT_THAT(
        rbobj_space(rbobj),
        Eq(32 - 1)); // 1 slot is always "used" to detect when buffer is full
    rbobj.mem->read = 5;
    rbobj.mem->write = 8;
    EXPECT_THAT(rbobj_space(rbobj), Eq(32 - 4));
    rbobj.mem->read = 8;
    rbobj.mem->write = 5;
    EXPECT_THAT(rbobj_space(rbobj), Eq(2));
    rbobj.mem->read = 16;
    rbobj.mem->write = 15;
    EXPECT_THAT(rbobj_space(rbobj), Eq(0));

    rbobj.mem->read = 0;
    rbobj.mem->write = 32 - 1;
    EXPECT_THAT(rbobj_space(rbobj), Eq(0));

    rbobj.mem->read = 32 - 1;
    rbobj.mem->write = 0;
    EXPECT_THAT(rbobj_space(rbobj), Eq(32 - 2));
}

#if 0
TEST_F(NAME, space_to_end_macro)
{
    ASSERT_THAT(rbobj_resize(&rbobj, 32), Eq(0));
    ASSERT_THAT(rbobj.mem->capacity, Eq(32));

    rbobj.mem->read = 0; rbobj.mem->write = 0; RB_SPACE_TO_END_N(result, rbobj, 32); EXPECT_THAT(result, Eq(32 - 1));
    rbobj.mem->read = 5; rbobj.mem->write = 8; RB_SPACE_TO_END_N(result, rbobj, 32); EXPECT_THAT(result, Eq(32 - 8));
    rbobj.mem->read = 8; rbobj.mem->write = 5; RB_SPACE_TO_END_N(result, rbobj, 32); EXPECT_THAT(result, Eq(2));
    rbobj.mem->read = 5; rbobj.mem->write = 5; RB_SPACE_TO_END_N(result, rbobj, 32); EXPECT_THAT(result, Eq(32 - 5));
    rbobj.mem->read = 6; rbobj.mem->write = 5; RB_SPACE_TO_END_N(result, rbobj, 32); EXPECT_THAT(result, Eq(0));

    rbobj.mem->read = 0; rbobj.mem->write = 32 - 1;
    RB_SPACE_TO_END_N(result, rbobj, 32);
    EXPECT_THAT(result, Eq(0));
}
#endif

TEST_F(NAME, is_full_and_is_empty_macros)
{
    rbobj.mem->read = 2;
    rbobj.mem->write = 2;
    EXPECT_THAT(rbobj_is_full(rbobj), IsFalse());
    EXPECT_THAT(rbobj_is_empty(rbobj), IsTrue());
    EXPECT_THAT(rbobj_is_full(rbobj), IsFalse());
    EXPECT_THAT(rbobj_is_empty(rbobj), IsTrue());

    rbobj.mem->read = 2;
    rbobj.mem->write = 3;
    EXPECT_THAT(rbobj_is_full(rbobj), IsFalse());
    EXPECT_THAT(rbobj_is_empty(rbobj), IsFalse());
    EXPECT_THAT(rbobj_is_full(rbobj), IsFalse());
    EXPECT_THAT(rbobj_is_empty(rbobj), IsFalse());

    rbobj.mem->read = 3;
    rbobj.mem->write = 2;
    EXPECT_THAT(rbobj_is_full(rbobj), IsTrue());
    EXPECT_THAT(rbobj_is_empty(rbobj), IsFalse());
    EXPECT_THAT(rbobj_is_full(rbobj), IsTrue());
    EXPECT_THAT(rbobj_is_empty(rbobj), IsFalse());

    rbobj.mem->read = 0;
    rbobj.mem->write = 32 - 1;
    EXPECT_THAT(rbobj_is_full(rbobj), IsTrue());
    EXPECT_THAT(rbobj_is_empty(rbobj), IsFalse());
    EXPECT_THAT(rbobj_is_full(rbobj), IsTrue());
    EXPECT_THAT(rbobj_is_empty(rbobj), IsFalse());

    rbobj.mem->read = 32 - 1;
    rbobj.mem->write = 0;
    EXPECT_THAT(rbobj_is_full(rbobj), IsFalse());
    EXPECT_THAT(rbobj_is_empty(rbobj), IsFalse());
    EXPECT_THAT(rbobj_is_full(rbobj), IsFalse());
    EXPECT_THAT(rbobj_is_empty(rbobj), IsFalse());
}

TEST_F(NAME, put)
{
    EXPECT_THAT(rbobj_count(rbobj), Eq(0));

    EXPECT_THAT(rbobj_put(rbobj, obj{0xa, 0xa, 0xa, 0xa}), Eq(0));
    EXPECT_THAT(rbobj_put(rbobj, obj{0xb, 0xb, 0xb, 0xb}), Eq(0));
    EXPECT_THAT(rbobj_put(rbobj, obj{0xc, 0xc, 0xc, 0xc}), Eq(0));

    EXPECT_THAT(rbobj.mem->read, Eq(0));
    EXPECT_THAT(rbobj_count(rbobj), Eq(3));
    EXPECT_THAT(rbobj.mem->write, Eq(3));
}

TEST_F(NAME, take)
{
    EXPECT_THAT(rbobj_put(rbobj, obj{0xa, 0xa, 0xa, 0xa}), Eq(0));
    EXPECT_THAT(rbobj_put(rbobj, obj{0xb, 0xb, 0xb, 0xb}), Eq(0));
    EXPECT_THAT(rbobj_put(rbobj, obj{0xc, 0xc, 0xc, 0xc}), Eq(0));

    ASSERT_THAT(rbobj_take(rbobj), Pointee(obj{0xa, 0xa, 0xa, 0xa}));
    ASSERT_THAT(rbobj_take(rbobj), Pointee(obj{0xb, 0xb, 0xb, 0xb}));
    ASSERT_THAT(rbobj_take(rbobj), Pointee(obj{0xc, 0xc, 0xc, 0xc}));

    EXPECT_THAT(rbobj_count(rbobj), Eq(0));
    EXPECT_THAT(rbobj.mem->read, Eq(3));
    EXPECT_THAT(rbobj.mem->write, Eq(3));
}

TEST_F(NAME, put_and_take_wrap)
{
    // We want to write an odd number of bytes to the ring buffer,
    // so all possible ways in which the pointers wrap are tested.
    // For this to work, the buffer size must be an even number.
    ASSERT_THAT(rbobj.mem->capacity % 2, Eq(0));

    for (int i = 0; i != rbobj.mem->capacity * 64; ++i)
    {
        // write 3 bytes
        EXPECT_THAT(rbobj_put(rbobj, obj{0xa, 0xa, 0xa, 0xa}), Eq(0));
        EXPECT_THAT(rbobj_put(rbobj, obj{0xb, 0xb, 0xb, 0xb}), Eq(0));
        EXPECT_THAT(rbobj_put(rbobj, obj{0xc, 0xc, 0xc, 0xc}), Eq(0));

        // read 3 bytes
        EXPECT_THAT(rbobj_take(rbobj), Pointee(obj{0xa, 0xa, 0xa, 0xa}));
        EXPECT_THAT(rbobj_take(rbobj), Pointee(obj{0xb, 0xb, 0xb, 0xb}));
        EXPECT_THAT(rbobj_take(rbobj), Pointee(obj{0xc, 0xc, 0xc, 0xc}));

        EXPECT_THAT(rbobj_count(rbobj), Eq(0));
        EXPECT_THAT(rbobj.mem->write, Eq(rbobj.mem->read));
        EXPECT_THAT(rbobj.mem->write, Lt(rbobj.mem->capacity));
        EXPECT_THAT(rbobj.mem->read, Lt(rbobj.mem->capacity));
    }
}

TEST_F(NAME, if_it_dont_fit_dont_shit_single)
{
    // We want to test the wraparound behavior of realloc, so move read/write
    // pointers to middle
    rbobj.mem->read = 16;
    rbobj.mem->write = 16;

    // Fill the buffer completely
    uint16_t num_left = 32 - 1;
    while (num_left--)
        ASSERT_THAT(
            rbobj_put(rbobj, obj{num_left, num_left, num_left, num_left}),
            Eq(0));

    ASSERT_THAT(rbobj_space(rbobj), Eq(0));
    ASSERT_THAT(rbobj_is_full(rbobj), IsTrue());
    ASSERT_THAT(rbobj.mem->read, Eq(16));
    ASSERT_THAT(rbobj.mem->write, Eq(15));

    // Write one entry, this should cause a realloc
    ASSERT_THAT(rbobj_put_realloc(&rbobj, obj{0xa, 0xa, 0xa, 0xa}), Eq(0));

    // Check to see if pointers make sense
    ASSERT_THAT(rbobj.mem->capacity, Eq(64));
    ASSERT_THAT(rbobj.mem->read, Eq(16));
    ASSERT_THAT(rbobj.mem->write, Eq(48));

    // Read everything back
    num_left = 32 - 1;
    while (num_left--)
        ASSERT_THAT(
            rbobj_take(rbobj),
            Pointee(obj{num_left, num_left, num_left, num_left}));

    ASSERT_THAT(rbobj_take(rbobj), Pointee(obj{0xa, 0xa, 0xa, 0xa}));

    ASSERT_THAT(rbobj_space(rbobj), Eq(rbobj.mem->capacity - 1));
    ASSERT_THAT(rbobj_is_empty(rbobj), IsTrue());
    ASSERT_THAT(rbobj.mem->read, Eq(48));
    ASSERT_THAT(rbobj.mem->write, Eq(48));
}

TEST_F(NAME, peek)
{
    // We want to test the wraparound behavior, so move read/write pointers to
    // middle
    rbobj.mem->read = 16;
    rbobj.mem->write = 16;

    // Fill the buffer completely
    uint16_t num_left = 32 - 1;
    while (num_left--)
        ASSERT_THAT(
            rbobj_put(rbobj, obj{num_left, num_left, num_left, num_left}),
            Eq(0));

    // Read everything back
    num_left = 32 - 1;
    for (int i = 0; i != rbobj_count(rbobj); ++i)
    {
        num_left--;
        ASSERT_THAT(
            rbobj_peek(rbobj, i),
            Pointee(obj{num_left, num_left, num_left, num_left}))
            << "num_left: " << num_left;
    }
}

TEST_F(NAME, for_each_with_wrap)
{
    // We want to test the wraparound behavior of realloc, so move read/write
    // pointers to middle
    rbobj.mem->read = 16;
    rbobj.mem->write = 16;

    // Fill the buffer completely
    uint16_t num_left = 32 - 1;
    while (num_left--)
        ASSERT_THAT(
            rbobj_put(rbobj, obj{num_left, num_left, num_left, num_left}),
            Eq(0));

    // Read everything back
    num_left = 32 - 1;
    struct obj* value;
    rb_for_each(rbobj, value)
    {
        num_left--;
        ASSERT_THAT(value, Pointee(obj{num_left, num_left, num_left, num_left}))
            << "num_left: " << num_left;
    }
}
