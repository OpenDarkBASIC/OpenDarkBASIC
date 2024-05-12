#include "odb-compiler/tests/DBParserTestHarness.hpp"
#include "odb-sdk/utf8.h"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/sdk/cmd_list.h"
}

#define NAME odbcompiler_command_list

using namespace testing;

class NAME : public Test
{
public:
    void
    SetUp() override
    {
        cmd_list_init(&cmds);
    }

    void
    TearDown() override
    {
        cmd_list_deinit(&cmds);
    }

    int
    addCommand(const char* name)
    {
        return cmd_list_add(
            &cmds,
            0,
            CMD_ARG_VOID,
            cstr_utf8_view(name),
            empty_utf8_view(),
            empty_utf8_view());
    }

    struct cmd_list cmds;
};

TEST_F(NAME, add_command_returns_ref)
{
    cmd_idx a = addCommand("projection matrix4");
    cmd_idx b = addCommand("randomize");
    cmd_idx c = addCommand("randomize matrix");

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
