#include "odb-sdk/utf8.h"

#include <gmock/gmock.h>

extern "C" {
#include "odb-sdk/utf8_list.h"
}

#define NAME odbsdk_utf8_list

using namespace testing;

class NAME : public Test
{
public:
    void
    SetUp() override
    {
        utf8_list_init(&l);
    }

    void
    TearDown() override
    {
        utf8_list_deinit(&l);
    }

    struct utf8_view
    U(const char* cstr)
    {
        return cstr_utf8_view(cstr);
    }

    struct utf8_ref
    R(const char* cstr)
    {
        return cstr_utf8_ref(cstr);
    }

    const char*
    C(struct utf8_view view)
    {
        return utf8_view_cstr(view);
    }

    struct utf8_list l;
};

TEST_F(NAME, add_one_string)
{
    ASSERT_THAT(utf8_list_add(&l, U("test")), Eq(0));
    ASSERT_THAT(utf8_list_count(&l), Eq(1));
    EXPECT_THAT(C(utf8_list_view(&l, 0)), StrEq("test"));
}

TEST_F(NAME, insert_one_string)
{
    ASSERT_THAT(utf8_list_insert_ref(&l, 0, "test", R("test")), Eq(0));
    ASSERT_THAT(utf8_list_count(&l), Eq(1));
    EXPECT_THAT(C(utf8_list_view(&l, 0)), StrEq("test"));
}

TEST_F(NAME, insert_string_depending_on_garbage_ref)
{
    utf8_list_add(&l, U("blub"));
    l.count = 0;
    l.str_used = 0;
    UTF8_LIST_TABLE_PTR(&l)[0].off = -666666;
    UTF8_LIST_TABLE_PTR(&l)[0].len = -999999;
    ASSERT_THAT(utf8_list_insert_ref(&l, 0, "test", R("test")), Eq(0));
    ASSERT_THAT(utf8_list_count(&l), Eq(1));
    EXPECT_THAT(C(utf8_list_view(&l, 0)), StrEq("test"));
}

TEST_F(NAME, add_strings_with_realloc)
{
    for (int i = 0; i != 32; ++i)
    {
        char buf[32];
        sprintf(buf, "test%d", i);
        ASSERT_THAT(utf8_list_add(&l, U(buf)), Eq(0));
    }

    ASSERT_THAT(utf8_list_count(&l), Eq(32));

    for (int i = 0; i != 32; ++i)
    {
        char buf[32];
        sprintf(buf, "test%d", i);
        ASSERT_THAT(C(utf8_list_view(&l, i)), StrEq(buf));
    }
}

TEST_F(NAME, insert_string_moves_memory_correctly)
{
    for (int i = 0; i != 32; ++i)
    {
        char buf[32];
        sprintf(buf, "test%d", i);
        ASSERT_THAT(utf8_list_add(&l, U(buf)), Eq(0));
    }

    ASSERT_THAT(
        utf8_list_insert_ref(&l, 8, "inserted string", R("inserted string")),
        Eq(0));
    ASSERT_THAT(utf8_list_count(&l), Eq(33));

    for (int i = 0; i != 8; ++i)
    {
        char buf[32];
        sprintf(buf, "test%d", i);
        ASSERT_THAT(C(utf8_list_view(&l, i)), StrEq(buf));
    }
    ASSERT_THAT(C(utf8_list_view(&l, 8)), StrEq("inserted string"));
    for (int i = 9; i != 32; ++i)
    {
        char buf[32];
        sprintf(buf, "test%d", i - 1);
        ASSERT_THAT(C(utf8_list_view(&l, i)), StrEq(buf));
    }
}

TEST_F(NAME, add_string_after_inserting_moves_memory_correctly)
{
    for (int i = 0; i != 32; ++i)
    {
        char buf[32];
        sprintf(buf, "test%d", i);
        ASSERT_THAT(utf8_list_add(&l, U(buf)), Eq(0));
    }

    ASSERT_THAT(
        utf8_list_insert_ref(&l, 8, "inserted string", R("inserted string")),
        Eq(0));
    ASSERT_THAT(utf8_list_add(&l, U("append1")), Eq(0));
    ASSERT_THAT(utf8_list_add(&l, U("append2")), Eq(0));
    ASSERT_THAT(utf8_list_count(&l), Eq(35));

    for (int i = 0; i != 8; ++i)
    {
        char buf[32];
        sprintf(buf, "test%d", i);
        ASSERT_THAT(C(utf8_list_view(&l, i)), StrEq(buf));
    }
    ASSERT_THAT(C(utf8_list_view(&l, 8)), StrEq("inserted string"));
    for (int i = 9; i != 32; ++i)
    {
        char buf[32];
        sprintf(buf, "test%d", i - 1);
        ASSERT_THAT(C(utf8_list_view(&l, i)), StrEq(buf));
    }
    ASSERT_THAT(C(utf8_list_view(&l, 33)), StrEq("append1"));
    ASSERT_THAT(C(utf8_list_view(&l, 34)), StrEq("append2"));
}
