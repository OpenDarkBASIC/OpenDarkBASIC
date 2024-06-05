#include "odb-compiler/tests/DBParserHelper.hpp"
#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/sdk/cmd_list.h"
}

#define NAME odbcompiler_cmd_list

using namespace testing;

struct NAME : DBParserHelper, Test
{
};

TEST_F(NAME, add_command_returns_ref)
{
    cmd_id a = addCommand("PROJECTION MATRIX4");
    cmd_id b = addCommand("RANDOMIZE");
    cmd_id c = addCommand("RANDOMIZE MATRIX");

    EXPECT_THAT(a, Eq(0));
    EXPECT_THAT(b, Eq(1));
    EXPECT_THAT(c, Eq(2));
}

TEST_F(NAME, added_commands_are_lexicographically_sorted)
{
    EXPECT_THAT(addCommand("RANDOMIZE MESH"), Eq(0));
    EXPECT_THAT(addCommand("RANDOMIZE"), Eq(0));
    EXPECT_THAT(addCommand("PROJECTION MATRIX4"), Eq(0));
    EXPECT_THAT(addCommand("RANDOMIZE MATRIX"), Eq(2));
    EXPECT_THAT(addCommand("READ"), Eq(4));
}

