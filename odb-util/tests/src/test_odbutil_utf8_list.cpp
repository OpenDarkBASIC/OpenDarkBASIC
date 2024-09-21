#include "odb-util/utf8.h"

#include <gmock/gmock.h>

extern "C" {
#include "odb-util/utf8_list.h"
}

#define NAME odbutil_utf8_list

using namespace testing;

class NAME : public Test
{
public:
    void
    SetUp() override
    {
        l = NULL;
    }

    void
    TearDown() override
    {
        utf8_list_deinit(l);
    }

    struct utf8_view
    U(const char* cstr)
    {
        return cstr_utf8_view(cstr);
    }

    struct utf8_list* l;
};

TEST_F(NAME, add_one_string)
{
    ASSERT_THAT(utf8_list_add(&l, U("test")), Eq(0));
    ASSERT_THAT(l->count, Eq(1));
    EXPECT_THAT(utf8_list_cstr(l, 0), StrEq("test"));
}

TEST_F(NAME, insert_one_string)
{
    ASSERT_THAT(utf8_list_insert(&l, 0, U("test")), Eq(0));
    ASSERT_THAT(l->count, Eq(1));
    EXPECT_THAT(utf8_list_cstr(l, 0), StrEq("test"));
}

TEST_F(NAME, insert_string_depending_on_garbage_ref)
{
    utf8_list_add(&l, U("blub"));
    l->count = 0;
    l->str_used = 0;
    UTF8_LIST_TABLE_PTR(l)[0].off = -666666;
    UTF8_LIST_TABLE_PTR(l)[0].len = -999999;
    ASSERT_THAT(utf8_list_insert(&l, 0, U("test")), Eq(0));
    ASSERT_THAT(l->count, Eq(1));
    EXPECT_THAT(utf8_list_cstr(l, 0), StrEq("test"));
}

TEST_F(NAME, add_strings_with_realloc)
{
    for (int i = 0; i != 32; ++i)
    {
        char buf[32];
        sprintf(buf, "test%d", i);
        ASSERT_THAT(utf8_list_add(&l, U(buf)), Eq(0));
    }

    ASSERT_THAT(l->count, Eq(32));

    for (int i = 0; i != 32; ++i)
    {
        char buf[32];
        sprintf(buf, "test%d", i);
        ASSERT_THAT(utf8_list_cstr(l, i), StrEq(buf));
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

    ASSERT_THAT(utf8_list_insert(&l, 8, U("inserted string")), Eq(0));
    ASSERT_THAT(l->count, Eq(33));

    for (int i = 0; i != 8; ++i)
    {
        char buf[32];
        sprintf(buf, "test%d", i);
        ASSERT_THAT(utf8_list_cstr(l, i), StrEq(buf));
    }
    ASSERT_THAT(utf8_list_cstr(l, 8), StrEq("inserted string"));
    for (int i = 9; i != 32; ++i)
    {
        char buf[32];
        sprintf(buf, "test%d", i - 1);
        ASSERT_THAT(utf8_list_cstr(l, i), StrEq(buf));
    }
}

TEST_F(NAME, erase_string_moves_memory_correctly)
{
    for (int i = 0; i != 32; ++i)
    {
        char buf[32];
        sprintf(buf, "test%d", i);
        ASSERT_THAT(utf8_list_add(&l, U(buf)), Eq(0));
    }

    utf8_list_erase(l, 8);
    ASSERT_THAT(l->count, Eq(31));

    for (int i = 0; i != 8; ++i)
    {
        char buf[32];
        sprintf(buf, "test%d", i);
        ASSERT_THAT(utf8_list_cstr(l, i), StrEq(buf));
    }
    for (int i = 8; i != 31; ++i)
    {
        char buf[32];
        sprintf(buf, "test%d", i + 1);
        ASSERT_THAT(utf8_list_cstr(l, i), StrEq(buf));
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

    ASSERT_THAT(utf8_list_insert(&l, 8, U("inserted string")), Eq(0));
    ASSERT_THAT(utf8_list_add(&l, U("append1")), Eq(0));
    ASSERT_THAT(utf8_list_add(&l, U("append2")), Eq(0));
    ASSERT_THAT(l->count, Eq(35));

    for (int i = 0; i != 8; ++i)
    {
        char buf[32];
        sprintf(buf, "test%d", i);
        ASSERT_THAT(utf8_list_cstr(l, i), StrEq(buf));
    }
    ASSERT_THAT(utf8_list_cstr(l, 8), StrEq("inserted string"));
    for (int i = 9; i != 32; ++i)
    {
        char buf[32];
        sprintf(buf, "test%d", i - 1);
        ASSERT_THAT(utf8_list_cstr(l, i), StrEq(buf));
    }
    ASSERT_THAT(utf8_list_cstr(l, 33), StrEq("append1"));
    ASSERT_THAT(utf8_list_cstr(l, 34), StrEq("append2"));
}
