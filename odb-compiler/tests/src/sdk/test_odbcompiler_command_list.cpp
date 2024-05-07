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

/*
TEST_F(NAME, exact_string)
{
    cmd::CommandIndex cmdIndex;
    cmdIndex.addCommand(new cmd::Command(
        nullptr, "projection matrix4", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(
        nullptr, "randomize", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(
        nullptr, "randomize matrix", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(
        nullptr, "randomize mesh", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(
        new cmd::Command(nullptr, "read", "", cmd::Command::Type::Void, {}));
    matcher->updateFromIndex(&cmdIndex);

    auto result = matcher->findLongestCommandMatching("randomize");

    EXPECT_THAT(result.found, IsTrue());
    EXPECT_THAT(result.matchedLength, Eq(9));
}

TEST_F(NAME, trailing_space)
{
    cmd::CommandIndex cmdIndex;
    cmdIndex.addCommand(new cmd::Command(nullptr, "projection matrix4", "",
cmd::Command::Type::Void, {})); cmdIndex.addCommand(new cmd::Command(nullptr,
"randomize", "", cmd::Command::Type::Void, {})); cmdIndex.addCommand(new
cmd::Command(nullptr, "randomize matrix", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(nullptr, "randomize mesh", "",
cmd::Command::Type::Void, {})); cmdIndex.addCommand(new cmd::Command(nullptr,
"read", "", cmd::Command::Type::Void, {})); matcher->updateFromIndex(&cmdIndex);

    auto result = matcher->findLongestCommandMatching("randomize ");

    EXPECT_THAT(result.found, IsTrue());
    EXPECT_THAT(result.matchedLength, Eq(9));
}

TEST_F(NAME, longer_symbol)
{
    cmd::CommandIndex cmdIndex;
    cmdIndex.addCommand(new cmd::Command(nullptr, "projection matrix4", "",
cmd::Command::Type::Void, {})); cmdIndex.addCommand(new cmd::Command(nullptr,
"randomize", "", cmd::Command::Type::Void, {})); cmdIndex.addCommand(new
cmd::Command(nullptr, "randomize matrix", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(nullptr, "randomize mesh", "",
cmd::Command::Type::Void, {})); cmdIndex.addCommand(new cmd::Command(nullptr,
"read", "", cmd::Command::Type::Void, {})); matcher->updateFromIndex(&cmdIndex);

    auto result = matcher->findLongestCommandMatching("randomized");

    EXPECT_THAT(result.found, IsFalse());
    EXPECT_THAT(result.matchedLength, Eq(9));
}

TEST_F(NAME, match_longer_string_to_shorter_command)
{
    cmd::CommandIndex cmdIndex;
    cmdIndex.addCommand(new cmd::Command(nullptr, "projection matrix4", "",
cmd::Command::Type::Void, {})); cmdIndex.addCommand(new cmd::Command(nullptr,
"randomize", "", cmd::Command::Type::Void, {})); cmdIndex.addCommand(new
cmd::Command(nullptr, "randomize matrix", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(nullptr, "randomize mesh", "",
cmd::Command::Type::Void, {})); cmdIndex.addCommand(new cmd::Command(nullptr,
"read", "", cmd::Command::Type::Void, {})); matcher->updateFromIndex(&cmdIndex);

    auto result = matcher->findLongestCommandMatching("randomize timer");

    EXPECT_THAT(result.found, IsTrue());
    EXPECT_THAT(result.matchedLength, Eq(9)); // strlen("randomize")
}

TEST_F(NAME, dont_match_shorter_string_to_longer_command)
{
    cmd::CommandIndex cmdIndex;
    cmdIndex.addCommand(new cmd::Command(nullptr, "dec", "",
cmd::Command::Type::Void, {})); matcher->updateFromIndex(&cmdIndex);

    auto result = matcher->findLongestCommandMatching("decalmax");

    EXPECT_THAT(result.found, IsFalse());
    EXPECT_THAT(result.matchedLength, Eq(3)); // strlen("dec")
}

TEST_F(NAME, match_when_multiple_options_include_spaces_and_non_spaces)
{
    cmd::CommandIndex cmdIndex;
    cmdIndex.addCommand(new cmd::Command(nullptr, "DELETE OBJECT COLLISION BOX",
"", cmd::Command::Type::Void, {})); cmdIndex.addCommand(new
cmd::Command(nullptr, "DELETE OBJECT", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(nullptr, "DELETE OBJECTS", "",
cmd::Command::Type::Void, {})); matcher->updateFromIndex(&cmdIndex);

    auto result = matcher->findLongestCommandMatching("delete object 100");

    EXPECT_THAT(result.found, IsTrue());
    EXPECT_THAT(result.matchedLength, Eq(strlen("delete object")));
}
*/
