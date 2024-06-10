#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-sdk/tests/LogHelper.hpp"

#include "gmock/gmock.h"

extern "C" {
}

#define NAME odbcompiler_cmd_matcher

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, empty_list)
{
    EXPECT_THAT(parse("RANDOMIZE"), Eq(-1));
}

TEST_F(NAME, exact_string)
{
    addCommand("PROJECTION MATRIX4");
    addCommand("RANDOMIZE");
    addCommand("RANDOMIZE MATRIX");
    addCommand("RANDOMIZE MESH");
    addCommand("READ");

    EXPECT_THAT(parse("RANDOMIZE"), Eq(0));
    int cmd = ast.nodes[0].block.stmt;
    EXPECT_THAT(ast.nodes[cmd].cmd.id, Eq(1));
}

TEST_F(NAME, trailing_space)
{
    addCommand("PROJECTION MATRIX4");
    addCommand("RANDOMIZE");
    addCommand("RANDOMIZE MATRIX");
    addCommand("RANDOMIZE MESH");
    addCommand("READ");

    EXPECT_THAT(parse("randomize "), Eq(0));
    int cmd = ast.nodes[0].block.stmt;
    EXPECT_THAT(ast.nodes[cmd].cmd.id, Eq(1));
}

TEST_F(NAME, longer_symbol)
{
    addCommand("PROJECTION MATRIX4");
    addCommand("RANDOMIZE");
    addCommand("RANDOMIZE MATRIX");
    addCommand("RANDOMIZE MESH");
    addCommand("READ");

    EXPECT_THAT(parse("randomized"), Eq(-1));
}

/*
TEST_F(NAME, match_longer_string_to_shorter_command)
{
    cmd::CommandIndex cmdIndex;
    cmdIndex.addCommand(new cmd::Command(nullptr, "PROJECTION MATRIX4", "",
cmd::Command::Type::Void, {})); cmdIndex.addCommand(new cmd::Command(nullptr,
"RANDOMIZE", "", cmd::Command::Type::Void, {})); cmdIndex.addCommand(new
cmd::Command(nullptr, "RANDOMIZE MATRIX", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(nullptr, "RANDOMIZE MESH", "",
cmd::Command::Type::Void, {})); cmdIndex.addCommand(new cmd::Command(nullptr,
"READ", "", cmd::Command::Type::Void, {})); matcher->updateFromIndex(&cmdIndex);

    auto result = matcher->findLongestCommandMatching("randomize timer");

    EXPECT_THAT(result.found, IsTrue());
    EXPECT_THAT(result.matchedLength, Eq(9)); // strlen("RANDOMIZE")
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
