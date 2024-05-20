#include "odb-compiler/tests/DBParserHelper.hpp"
#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/sdk/cmd_list.h"
}

#define NAME odbcompiler_cmd_list

using namespace testing;

struct NAME : DBParserHelper
{
};

TEST_F(NAME, add_command_returns_ref)
{
    cmd_id a = addCommand("projection matrix4");
    cmd_id b = addCommand("randomize");
    cmd_id c = addCommand("randomize matrix");

    EXPECT_THAT(a, Eq(0));
    EXPECT_THAT(b, Eq(1));
    EXPECT_THAT(c, Eq(2));
}

TEST_F(NAME, added_commands_are_lexicographically_sorted)
{
    EXPECT_THAT(addCommand("randomize mesh"), Eq(0));
    EXPECT_THAT(addCommand("randomize"), Eq(0));
    EXPECT_THAT(addCommand("projection matrix4"), Eq(0));
    EXPECT_THAT(addCommand("randomize matrix"), Eq(2));
    EXPECT_THAT(addCommand("read"), Eq(4));
}

