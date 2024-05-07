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
    } const char*
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

TEST_F(NAME, add_strings_until_realloc)
{
    for (int i = 0; i != 256; ++i)
    {
        char buf[32];
        sprintf(buf, "test%d", i);
        ASSERT_THAT(utf8_list_add(&l, U(buf)), Eq(0));
    }

    ASSERT_THAT(utf8_list_count(&l), Eq(256));

    for (int i = 0; i != 256; ++i)
    {
        char buf[32];
        sprintf(buf, "test%d", i);
        ASSERT_THAT(C(utf8_list_view(&l, i)), StrEq(buf));
    }
}

TEST_F(NAME, insert_strings_until_realloc)
{
    for (int i = 0; i != 256; ++i)
    {
        char buf[32];
        sprintf(buf, "test%d", i);
        ASSERT_THAT(utf8_list_insert(&l, i/2, U(buf)), Eq(0));
    }

    ASSERT_THAT(utf8_list_count(&l), Eq(256));
    
    for (int i = 0; i != 256; ++i)
    {
        char buf[32];
        sprintf(buf, "test%d", i);
        //ASSERT_THAT(C(utf8_list_view(&l, i)), StrEq(buf));
        puts(C(utf8_list_view(&l, i)));
    }
}
