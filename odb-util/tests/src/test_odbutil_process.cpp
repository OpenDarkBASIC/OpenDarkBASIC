#include "odb-util/tests/Utf8Helper.hpp"

#include <gmock/gmock.h>

extern "C" {
#include "odb-util/process.h"
}

#define NAME odbutil_process

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

TEST_F(NAME, exit_code)
{
#if defined(_WIN32)
    const char* argv[] = {"odb-echo.exe", "--exit", "42", NULL};
#else
    const char* argv[] = {"./odb-echo", "--exit", "42", NULL};
#endif
    ASSERT_THAT(
        process_run(
            cstr_ospathc(argv[0]), empty_ospathc(), argv, empty_utf8_view(), &out, &err, 0),
        Eq(42));
    ASSERT_THAT(out, Utf8Eq(""));
    ASSERT_THAT(err, Utf8Eq(""));
}

TEST_F(NAME, echo_stdout)
{
#if defined(_WIN32)
    const char* argv[] = {"odb-echo.exe", NULL};
#else
    const char* argv[] = {"./odb-echo", NULL};
#endif
    ASSERT_THAT(
        process_run(
            cstr_ospathc(argv[0]),
            empty_ospathc(),
            argv,
            cstr_utf8_view("This is a test"),
            &out,
            &err,
            0),
        Eq(0));
    ASSERT_THAT(out, Utf8Eq("This is a test"));
    ASSERT_THAT(err, Utf8Eq(""));
}

TEST_F(NAME, echo_stderr)
{
#if defined(_WIN32)
    const char* argv[] = {"odb-echo.exe", "--stderr", NULL};
#else
    const char* argv[] = {"./odb-echo", "--stderr", NULL};
#endif
    ASSERT_THAT(
        process_run(
            cstr_ospathc(argv[0]),
            empty_ospathc(),
            argv,
            cstr_utf8_view("This is a test"),
            &out,
            &err,
            0),
        Eq(0));
    ASSERT_THAT(out, Utf8Eq(""));
    ASSERT_THAT(err, Utf8Eq("This is a test"));
}
