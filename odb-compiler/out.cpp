#include <gmock/gmock.h>

#include "odb-sdk/tests/Utf8Helper.hpp"
extern "C" {
#include "odb-sdk/process.h"
#include "odb-sdk/utf8.h"
}

#define NAME odbcompiler_ci

using namespace testing;

struct NAME : Test
{
    void SetUp() override
    {
        out = empty_utf8();
        err = empty_utf8();
    }
    void TearDown() override
    {
        utf8_deinit(out);
        utf8_deinit(err);
    }
    struct utf8 out, err;
};

TEST_F(NAME, cy_ry_01_pow_over_div)
{
    const char* argv[] = {
        "./odb-cli",
        "-b",
        "--dba",
        "--output",
        "cy_ry_01_pow_over_div",
        NULL
    };
    ASSERT_THAT(process_run(
        cstr_ospathc("./odb-cli"),
        argv,
        cstr_utf8_view(
            "if a() / b() ^ c() then x=1\n"
            "end\n"
            "\n"
            "function a()\n"
            "    print \"a\"\n"
            "endfunction 1\n"
            "function b()\n"
            "    print \"b\"\n"
            "endfunction 1\n"
            "function c()\n"
            "    print \"c\"\n"
            "endfunction 1\n"
            "\n"
            ""),
        &out, &err), Eq(0));
    ASSERT_THAT(out, Utf8Eq("bca\n"
));
    ASSERT_THAT(err, Utf8Eq(""));
}

