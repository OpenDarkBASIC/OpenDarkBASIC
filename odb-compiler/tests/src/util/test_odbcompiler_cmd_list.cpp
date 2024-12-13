#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/sdk/cmd_list.h"
}

#define NAME odbcompiler_cmd_list

using namespace testing;

struct NAME : Test
{
    void
    SetUp() override
    {
        plugin_list_init(&plugins);
        cmd_list_init(&cmds);
    }

    void
    TearDown() override
    {
        cmd_list_deinit(&cmds);
        plugin_list_deinit(plugins);
    }

    cmd_id
    addCommand(
        enum primitive_type                        ret_type,
        const char*                                name,
        std::initializer_list<enum primitive_type> param_types)
    {
        cmd_id cmd = cmd_list_add(
            &cmds,
            0,
            primitive_type(ret_type),
            cstr_utf8_view(name),
            empty_utf8_view());
        if (cmd < 0)
            return cmd;

        for (enum primitive_type type : param_types)
            if (cmd_add_param(
                    &cmds,
                    cmd,
                    primitive_type(type),
                    CMD_PARAM_IN,
                    cstr_utf8_view(primitive_type_name(type)))
                < 0)
            {
                cmd_list_erase(&cmds, cmd);
                return -1;
            }

        return cmd;
    }

    struct plugin_list* plugins;
    struct cmd_list     cmds;
};

TEST_F(NAME, add_command_returns_ref)
{
    cmd_id a = addCommand(TYPE_VOID, "PROJECTION MATRIX4", {});
    cmd_id b = addCommand(TYPE_VOID, "RANDOMIZE", {});
    cmd_id c = addCommand(TYPE_VOID, "RANDOMIZE MATRIX", {});

    EXPECT_THAT(a, Eq(0));
    EXPECT_THAT(b, Eq(1));
    EXPECT_THAT(c, Eq(2));
}

TEST_F(NAME, added_commands_are_lexicographically_sorted)
{
    EXPECT_THAT(addCommand(TYPE_VOID, "RANDOMIZE MESH", {}), Eq(0));
    EXPECT_THAT(addCommand(TYPE_VOID, "RANDOMIZE", {}), Eq(0));
    EXPECT_THAT(addCommand(TYPE_VOID, "PROJECTION MATRIX4", {}), Eq(0));
    EXPECT_THAT(addCommand(TYPE_VOID, "RANDOMIZE MATRIX", {}), Eq(2));
    EXPECT_THAT(addCommand(TYPE_VOID, "READ", {}), Eq(4));
}
