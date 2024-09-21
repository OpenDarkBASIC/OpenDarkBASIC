#include "gmock/gmock.h"

extern "C" {
#include "odb-util/log.h"
#include "odb-util/mem.h"
#include "odb-util/rb.h"
}

struct obj
{
    uint64_t a, b, c, d;
};

bool
operator==(const struct obj& a, const struct obj& b)
{
    return a.a == b.a && a.b == b.b && a.c == b.c && a.d == b.d;
}

RB_DECLARE_API(static, rbobj, struct obj, 16)
RB_DEFINE_API(rbobj, struct obj, 16)

#define NAME odbutil_rb

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
        rbobj_deinit(rbobj);
    }

    struct rbobj* rbobj;
};

TEST_F(NAME, resize_initializes_members)
{
    rbobj_deinit(rbobj);
    rbobj_init(&rbobj);
    ASSERT_THAT(rbobj_resize(&rbobj, 32), Eq(0));
    ASSERT_THAT(rbobj, NotNull());
    ASSERT_THAT(rbobj->capacity, Eq(32));
    ASSERT_THAT(rbobj->read, Eq(0));
    ASSERT_THAT(rbobj->write, Eq(0));
}

TEST_F(NAME, space_left)
{
    rbobj->read = 0;
    rbobj->write = 0;
    EXPECT_THAT(
        rbobj_space(rbobj),
        Eq(32 - 1)); // 1 slot is always "used" to detect when buffer is full
    rbobj->read = 5;
    rbobj->write = 8;
    EXPECT_THAT(rbobj_space(rbobj), Eq(32 - 4));
    rbobj->read = 8;
    rbobj->write = 5;
    EXPECT_THAT(rbobj_space(rbobj), Eq(2));
    rbobj->read = 16;
    rbobj->write = 15;
    EXPECT_THAT(rbobj_space(rbobj), Eq(0));

    rbobj->read = 0;
    rbobj->write = 32 - 1;
    EXPECT_THAT(rbobj_space(rbobj), Eq(0));

    rbobj->read = 32 - 1;
    rbobj->write = 0;
    EXPECT_THAT(rbobj_space(rbobj), Eq(32 - 2));
}

TEST_F(NAME, is_full_and_is_empty_macros)
{
    rbobj->read = 2;
    rbobj->write = 2;
    EXPECT_THAT(rbobj_is_full(rbobj), IsFalse());
    EXPECT_THAT(rbobj_is_empty(rbobj), IsTrue());
    EXPECT_THAT(rbobj_is_full(rbobj), IsFalse());
    EXPECT_THAT(rbobj_is_empty(rbobj), IsTrue());

    rbobj->read = 2;
    rbobj->write = 3;
    EXPECT_THAT(rbobj_is_full(rbobj), IsFalse());
    EXPECT_THAT(rbobj_is_empty(rbobj), IsFalse());
    EXPECT_THAT(rbobj_is_full(rbobj), IsFalse());
    EXPECT_THAT(rbobj_is_empty(rbobj), IsFalse());

    rbobj->read = 3;
    rbobj->write = 2;
    EXPECT_THAT(rbobj_is_full(rbobj), IsTrue());
    EXPECT_THAT(rbobj_is_empty(rbobj), IsFalse());
    EXPECT_THAT(rbobj_is_full(rbobj), IsTrue());
    EXPECT_THAT(rbobj_is_empty(rbobj), IsFalse());

    rbobj->read = 0;
    rbobj->write = 32 - 1;
    EXPECT_THAT(rbobj_is_full(rbobj), IsTrue());
    EXPECT_THAT(rbobj_is_empty(rbobj), IsFalse());
    EXPECT_THAT(rbobj_is_full(rbobj), IsTrue());
    EXPECT_THAT(rbobj_is_empty(rbobj), IsFalse());

    rbobj->read = 32 - 1;
    rbobj->write = 0;
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

    EXPECT_THAT(rbobj->read, Eq(0));
    EXPECT_THAT(rbobj_count(rbobj), Eq(3));
    EXPECT_THAT(rbobj->write, Eq(3));
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
    EXPECT_THAT(rbobj->read, Eq(3));
    EXPECT_THAT(rbobj->write, Eq(3));
}

TEST_F(NAME, put_and_take_wrap)
{
    // We want to write an odd number of bytes to the ring buffer,
    // so all possible ways in which the pointers wrap are tested.
    // For this to work, the buffer size must be an even number.
    ASSERT_THAT(rbobj->capacity % 2, Eq(0));

    for (int i = 0; i != rbobj->capacity * 64; ++i)
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
        EXPECT_THAT(rbobj->write, Eq(rbobj->read));
        EXPECT_THAT(rbobj->write, Lt(rbobj->capacity));
        EXPECT_THAT(rbobj->read, Lt(rbobj->capacity));
    }
}

TEST_F(NAME, if_it_dont_fit_dont_shit_single)
{
    // We want to test the wraparound behavior of realloc, so move read/write
    // pointers to middle
    rbobj->read = 16;
    rbobj->write = 16;

    // Fill the buffer completely
    uint16_t num_left = 32 - 1;
    while (num_left--)
        ASSERT_THAT(
            rbobj_put(rbobj, obj{num_left, num_left, num_left, num_left}),
            Eq(0));

    ASSERT_THAT(rbobj_space(rbobj), Eq(0));
    ASSERT_THAT(rbobj_is_full(rbobj), IsTrue());
    ASSERT_THAT(rbobj->read, Eq(16));
    ASSERT_THAT(rbobj->write, Eq(15));

    // Write one entry, this should cause a realloc
    ASSERT_THAT(rbobj_put_realloc(&rbobj, obj{0xa, 0xa, 0xa, 0xa}), Eq(0));

    // Check to see if pointers make sense
    ASSERT_THAT(rbobj->capacity, Eq(64));
    ASSERT_THAT(rbobj->read, Eq(16));
    ASSERT_THAT(rbobj->write, Eq(48));

    // Read everything back
    num_left = 32 - 1;
    while (num_left--)
        ASSERT_THAT(
            rbobj_take(rbobj),
            Pointee(obj{num_left, num_left, num_left, num_left}));

    ASSERT_THAT(rbobj_take(rbobj), Pointee(obj{0xa, 0xa, 0xa, 0xa}));

    ASSERT_THAT(rbobj_space(rbobj), Eq(rbobj->capacity - 1));
    ASSERT_THAT(rbobj_is_empty(rbobj), IsTrue());
    ASSERT_THAT(rbobj->read, Eq(48));
    ASSERT_THAT(rbobj->write, Eq(48));
}

TEST_F(NAME, peek)
{
    // We want to test the wraparound behavior, so move read/write pointers to
    // middle
    rbobj->read = 16;
    rbobj->write = 16;

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
    rbobj->read = 16;
    rbobj->write = 16;

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
