#include <gmock/gmock.h>

extern "C" {
#include "odb-sdk/utf8.h"
}

#define NAME odbsdk_utf8

using namespace testing;

namespace {
class NAME : public Test
{
public:
    void
    SetUp() override
    {
    }
    void
    TearDown() override
    {
        utf8_deinit(str);
    }

    struct utf8 str = utf8();
};
} // namespace

TEST_F(NAME, set_empty_string)
{
    utf8_set_cstr(&str, "");
    EXPECT_THAT(str.len, Eq(0));
    EXPECT_THAT(str.data, NotNull());
    EXPECT_THAT(utf8_cstr(str), StrEq(""));
}

TEST_F(NAME, can_append_null_bytes)
{
    struct utf8_view eob = {"", 0, 1};
    utf8_set_cstr(&str, "test");
    EXPECT_THAT(str.len, Eq(4));

    utf8_append(&str, eob);
    const char* cstr = utf8_cstr(str);
    EXPECT_THAT(str.len, Eq(5));
    EXPECT_THAT(cstr[4], Eq('\0'));
    EXPECT_THAT(cstr[5], Eq('\0'));
}

TEST_F(NAME, format)
{
    utf8_fmt(&str, "%s, %d, %x", "test", 42, 0xB00B);
    EXPECT_THAT(str.len, Eq(14));
    EXPECT_THAT(str.data, StrEq("test, 42, B00B"));
}

TEST_F(NAME, utf16_to_utf8_weirdness)
{
    std::u16string    u16 = u"xxxxMay";
    struct utf16_view in
        = {(const uint16_t*)u16.data() + 4, (utf16_idx)u16.length() - 4};

    struct utf8 out = empty_utf8();
    utf16_to_utf8(&out, in);

    EXPECT_THAT(utf8_cstr(out), StrEq("May"));
    utf8_deinit(out);
}
