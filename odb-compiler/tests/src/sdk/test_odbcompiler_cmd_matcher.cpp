#include "odb-sdk/utf8.h"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/sdk/cmd_list.h"
}

#define NAME odbcompiler_cmd_matcher

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

    struct utf8_span
    R(const char* cstr)
    {
        return cstr_utf8_span(cstr);
    }

    cmd_idx
    addCommand(const char* cstr)
    {
        return cmd_list_add(
            &cmds,
            0,
            CMD_PARAM_VOID,
            cstr_utf8_view(cstr),
            empty_utf8_view(),
            empty_utf8_view());
    }

    utf8_idx
    matchCommand(const char* cstr)
    {
        return cmd_list_find(&cmds, cstr_utf8_view(cstr));
    }

    struct cmd_list cmds;
};

TEST_F(NAME, empty_list)
{
    EXPECT_THAT(matchCommand("randomize"), Eq(0));
}

TEST_F(NAME, exact_string)
{
    addCommand("projection matrix4");
    addCommand("randomize");
    addCommand("randomize matrix");
    addCommand("randomize mesh");
    addCommand("read");

    EXPECT_THAT(matchCommand("randomize"), Eq(9));
}
/*
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
}*/

TEST_F(NAME, longer_symbol)
{
    addCommand("projection matrix4");
    addCommand("randomize");
    addCommand("randomize matrix");
    addCommand("randomize mesh");
    addCommand("read");

    EXPECT_THAT(matchCommand("randomized"), Eq(0));
}

/*
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
