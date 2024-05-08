#include "odb-sdk/utf8.h"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/sdk/command_list.h"
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

    struct utf8_view
    U(const char* cstr)
    {
        return cstr_utf8_view(cstr);
    }
    const char*
    C(struct utf8_view str)
    {
        return utf8_view_cstr(str);
    }

    struct cmd_list cmds;
};

TEST_F(NAME, add_command_returns_ref)
{
    cmd_ref a = cmd_list_add(&cmds, 0, CMD_ARG_VOID, U("projection matrix4"), U(""), U(""));
    cmd_ref b = cmd_list_add(&cmds, 0, CMD_ARG_VOID, U("randomize"), U(""), U(""));
    cmd_ref c = cmd_list_add(&cmds, 0, CMD_ARG_VOID, U("randomize matrix"), U(""), U(""));

    EXPECT_THAT(a, Eq(0));
    EXPECT_THAT(b, Eq(1));
    EXPECT_THAT(c, Eq(2));
}

TEST_F(NAME, added_commands_are_lexicographically_sorted)
{
    EXPECT_THAT(cmd_list_add(&cmds, 0, CMD_ARG_VOID, U("randomize mesh"), U(""), U("")), Eq(0));
    EXPECT_THAT(cmd_list_add(&cmds, 0, CMD_ARG_VOID, U("randomize"), U(""), U("")), Eq(0));
    EXPECT_THAT(cmd_list_add(&cmds, 0, CMD_ARG_VOID, U("projection matrix4"), U(""), U("")), Eq(0));
    EXPECT_THAT(cmd_list_add(&cmds, 0, CMD_ARG_VOID, U("randomize matrix"), U(""), U("")), Eq(2));
    EXPECT_THAT(cmd_list_add(&cmds, 0, CMD_ARG_VOID, U("read"), U(""), U("")), Eq(4));
}

