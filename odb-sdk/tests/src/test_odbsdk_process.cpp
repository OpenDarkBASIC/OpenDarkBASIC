#include <gmock/gmock.h>
#include "odb-sdk/tests/Utf8Helper.hpp"

extern "C" {
#include "odb-sdk/process.h"
}

#define NAME odbsdk_process

using namespace testing;

struct NAME : public Test
{
    void
    SetUp() override
    {
        out = empty_utf8();
        err = empty_utf8();
    }
    void
    TearDown() override
    {
        utf8_deinit(out);
        utf8_deinit(err);
    }
    struct utf8 out, err;
};

TEST_F(NAME, run_echo)
{
    const char* argv[] = {"./odb-echo", NULL};
    ASSERT_THAT(
        process_run(
            cstr_ospathc("./odb-echo"),
            argv,
            cstr_utf8_view("This is a test"),
            &out,
            &err),
        Eq(0));
    ASSERT_THAT(out, Utf8Eq("This is a test"));
    ASSERT_THAT(err, Utf8Eq(""));
}
