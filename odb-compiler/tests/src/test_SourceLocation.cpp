#include "gmock/gmock.h"
#include "odb-compiler/ast/SourceLocation.hpp"

#define NAME SourceLocation

using namespace testing;
using namespace odb::ast;

TEST(NAME, empty_location)
{
    InlineSourceLocation sl("test", "", 1, 1, 1, 2);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:1"));
}

TEST(NAME, single_char_location)
{
    InlineSourceLocation sl("test", "some command 1, 2, 3", 1, 1, 2, 3);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:2"));

    std::vector<std::string> sh = sl.getUnderlinedSection();
    EXPECT_THAT(sh[0], StrEq("some command 1, 2, 3"));
    EXPECT_THAT(sh[1], StrEq(" ^"));
}

TEST(NAME, multi_char_location_1)
{
    InlineSourceLocation sl("test", "some command 1, 2, 3", 1, 1, 4, 8);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:4"));

    std::vector<std::string> sh = sl.getUnderlinedSection();
    EXPECT_THAT(sh[0], StrEq("some command 1, 2, 3"));
    EXPECT_THAT(sh[1], StrEq("   ^~~~"));
}

TEST(NAME, multi_char_location_2)
{
    InlineSourceLocation sl("test", "some command 1, 2, 3\nanother command 4, 5, 6\n", 2, 2, 4, 8);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:2:4"));

    std::vector<std::string> sh = sl.getUnderlinedSection();
    EXPECT_THAT(sh[0], StrEq("another command 4, 5, 6"));
    EXPECT_THAT(sh[1], StrEq("   ^~~~"));
}

TEST(NAME, multi_line_location_1)
{
    InlineSourceLocation sl("test", "some command 1, 2, 3\nanother command 4, 5, 6\n", 1, 2, 4, 8);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:4"));

    std::vector<std::string> sh = sl.getUnderlinedSection();
    EXPECT_THAT(sh[0], StrEq("some command 1, 2, 3"));
    EXPECT_THAT(sh[1], StrEq("   ^~~~~~~~~~~~~~~~~"));
    EXPECT_THAT(sh[2], StrEq("another command 4, 5, 6"));
    EXPECT_THAT(sh[3], StrEq("~~~~~~~"));
}

TEST(NAME, multi_line_location_2)
{
    InlineSourceLocation sl("test", "some command 1, 2, 3\nanother command 4, 5, 6\n", 1, 2, 9, 8);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:9"));

    std::vector<std::string> sh = sl.getUnderlinedSection();
    EXPECT_THAT(sh[0], StrEq("some command 1, 2, 3"));
    EXPECT_THAT(sh[1], StrEq("        ^~~~~~~~~~~~"));
    EXPECT_THAT(sh[2], StrEq("another command 4, 5, 6"));
    EXPECT_THAT(sh[3], StrEq("~~~~~~~"));
}

TEST(NAME, multi_line_location_3)
{
    InlineSourceLocation sl("test", "some command 1, 2, 3\nanother command 4, 5, 6\nyet another command 7, 8, 9",
                            1, 3, 4, 8);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:4"));

    std::vector<std::string> sh = sl.getUnderlinedSection();
    EXPECT_THAT(sh[0], StrEq("some command 1, 2, 3"));
    EXPECT_THAT(sh[1], StrEq("   ^~~~~~~~~~~~~~~~~"));
    EXPECT_THAT(sh[2], StrEq("another command 4, 5, 6"));
    EXPECT_THAT(sh[3], StrEq("~~~~~~~~~~~~~~~~~~~~~~~"));
    EXPECT_THAT(sh[4], StrEq("yet another command 7, 8, 9"));
    EXPECT_THAT(sh[5], StrEq("~~~~~~~"));
}

TEST(NAME, multi_line_location_4)
{
    InlineSourceLocation sl("test", "some command 1, 2, 3\nanother command 4, 5, 6\nyet another command 7, 8, 9",
                            1, 3, 7, 5);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:7"));

    std::vector<std::string> sh = sl.getUnderlinedSection();
    EXPECT_THAT(sh[0], StrEq("some command 1, 2, 3"));
    EXPECT_THAT(sh[1], StrEq("      ^~~~~~~~~~~~~~"));
    EXPECT_THAT(sh[2], StrEq("another command 4, 5, 6"));
    EXPECT_THAT(sh[3], StrEq("~~~~~~~~~~~~~~~~~~~~~~~"));
    EXPECT_THAT(sh[4], StrEq("yet another command 7, 8, 9"));
    EXPECT_THAT(sh[5], StrEq("~~~~"));
}

TEST(NAME, tabs_and_spaces_1)
{
    InlineSourceLocation sl("test", "  \t    some command 1, 2, 3", 1, 1, 8, 12);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:8"));

    std::vector<std::string> sh = sl.getUnderlinedSection();
    EXPECT_THAT(sh[0], StrEq("          some command 1, 2, 3"));
    EXPECT_THAT(sh[1], StrEq("          ^~~~"));
}

TEST(NAME, tabs_and_spaces_2)
{
    InlineSourceLocation sl("test", "      \tsome command 1, 2, 3", 1, 1, 8, 12);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:8"));

    std::vector<std::string> sh = sl.getUnderlinedSection();
    EXPECT_THAT(sh[0], StrEq("          some command 1, 2, 3"));
    EXPECT_THAT(sh[1], StrEq("          ^~~~"));
}

TEST(NAME, tabs_and_spaces_3)
{
    InlineSourceLocation sl("test", "  \t    some\tcommand 1, 2, 3", 1, 1, 7, 14);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:7"));

    std::vector<std::string> sh = sl.getUnderlinedSection();
    EXPECT_THAT(sh[0], StrEq("          some    command 1, 2, 3"));
    EXPECT_THAT(sh[1], StrEq("         ^~~~~~~~~~"));
}

TEST(NAME, tabs_and_spaces_4)
{
    InlineSourceLocation sl("test", "\t  some command 1, 2, 3\n\tanother command 4, 5, 6\n\t\tyet another command 7, 8, 9",
                            1, 3, 9, 6);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:9"));

    std::vector<std::string> sh = sl.getUnderlinedSection();
    EXPECT_THAT(sh[0], StrEq("      some command 1, 2, 3"));
    EXPECT_THAT(sh[1], StrEq("           ^~~~~~~~~~~~~~~"));
    EXPECT_THAT(sh[2], StrEq("    another command 4, 5, 6"));
    EXPECT_THAT(sh[3], StrEq("    ~~~~~~~~~~~~~~~~~~~~~~~"));
    EXPECT_THAT(sh[4], StrEq("        yet another command 7, 8, 9"));
    EXPECT_THAT(sh[5], StrEq("        ~~~"));
}

TEST(NAME, tabs_and_spaces_5)
{
    InlineSourceLocation sl("test", "  \tsome command 1, 2, 3\n\tanother\tcommand 4, 5, 6\n\t\tyet\tanother command 7, 8, 9",
                            1, 3, 3, 6);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:3"));

    std::vector<std::string> sh = sl.getUnderlinedSection();
    EXPECT_THAT(sh[0], StrEq("      some command 1, 2, 3"));
    EXPECT_THAT(sh[1], StrEq("  ^~~~~~~~~~~~~~~~~~~~~~~~"));
    EXPECT_THAT(sh[2], StrEq("    another    command 4, 5, 6"));
    EXPECT_THAT(sh[3], StrEq("    ~~~~~~~~~~~~~~~~~~~~~~~~~~"));
    EXPECT_THAT(sh[4], StrEq("        yet    another command 7, 8, 9"));
    EXPECT_THAT(sh[5], StrEq("        ~~~"));
}

TEST(NAME, tabs_and_spaces_6)
{
    InlineSourceLocation sl("test", "  \tsome command 1, 2, 3\n\tanother\tcommand 4, 5, 6\t\n\t\tyet\tanother command 7, 8, 9",
                            1, 3, 4, 6);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:4"));

    std::vector<std::string> sh = sl.getUnderlinedSection();
    EXPECT_THAT(sh[0], StrEq("      some command 1, 2, 3"));
    EXPECT_THAT(sh[1], StrEq("      ^~~~~~~~~~~~~~~~~~~~"));
    EXPECT_THAT(sh[2], StrEq("    another    command 4, 5, 6    "));
    EXPECT_THAT(sh[3], StrEq("    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
    EXPECT_THAT(sh[4], StrEq("        yet    another command 7, 8, 9"));
    EXPECT_THAT(sh[5], StrEq("        ~~~"));
}

TEST(NAME, invalid_location_1)
{
    InlineSourceLocation sl("test", "lol", 1, 1, 4, 5);
    std::vector<std::string> sh = sl.getUnderlinedSection();
    EXPECT_THAT(sh[0], StrEq("(Invalid location 1,1,4,5)"));
}

TEST(NAME, invalid_location_2)
{
    InlineSourceLocation sl("test", "lol", 1, 1, 3, 5);
    std::vector<std::string> sh = sl.getUnderlinedSection();
    EXPECT_THAT(sh[0], StrEq("(Invalid location 1,1,3,5)"));
}

TEST(NAME, invalid_location_3)
{
    InlineSourceLocation sl("test", "lol", 2, 2, 2, 3);
    std::vector<std::string> sh = sl.getUnderlinedSection();
    EXPECT_THAT(sh[0], StrEq("(Invalid location 2,2,2,3)"));
}

TEST(NAME, invalid_location_4)
{
    InlineSourceLocation sl("test", "lol", 1, 2, 2, 3);
    std::vector<std::string> sh = sl.getUnderlinedSection();
    EXPECT_THAT(sh[0], StrEq("(Invalid location 1,2,2,3)"));
}

TEST(NAME, invalid_location_5)
{
    InlineSourceLocation sl("test", "lol\nlel\nlil", 1, 3, 4, 3);
    std::vector<std::string> sh = sl.getUnderlinedSection();
    EXPECT_THAT(sh[0], StrEq("(Invalid location 1,3,4,3)"));
}

TEST(NAME, invalid_location_6)
{
    InlineSourceLocation sl("test", "lol\nlel\nlil", 1, 3, 2, 5);
    std::vector<std::string> sh = sl.getUnderlinedSection();
    EXPECT_THAT(sh[0], StrEq("(Invalid location 1,3,2,5)"));
}

TEST(NAME, empty_lines)
{
    InlineSourceLocation sl("test", "some command 1, 2, 3\n\n\nanother command 4, 5, 6\nyet another command 7, 8, 9",
                            1, 5, 4, 8);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:4"));

    std::vector<std::string> sh = sl.getUnderlinedSection();
    EXPECT_THAT(sh[0], StrEq("some command 1, 2, 3"));
    EXPECT_THAT(sh[1], StrEq("   ^~~~~~~~~~~~~~~~~"));
    EXPECT_THAT(sh[2], StrEq(""));
    EXPECT_THAT(sh[3], StrEq(""));
    EXPECT_THAT(sh[4], StrEq(""));
    EXPECT_THAT(sh[5], StrEq(""));
    EXPECT_THAT(sh[6], StrEq("another command 4, 5, 6"));
    EXPECT_THAT(sh[7], StrEq("~~~~~~~~~~~~~~~~~~~~~~~"));
    EXPECT_THAT(sh[8], StrEq("yet another command 7, 8, 9"));
    EXPECT_THAT(sh[9], StrEq("~~~~~~~"));
}

TEST(NAME, unionize_columns_1)
{
    InlineSourceLocation sl1("test", "some test string", 1, 1, 5, 8);
    InlineSourceLocation sl2("test", "some test string", 1, 1, 6, 10);
    sl1.unionize(&sl2);
    EXPECT_THAT(sl1.firstColumn(), Eq(5));
    EXPECT_THAT(sl1.lastColumn(), Eq(10));
}

TEST(NAME, unionize_columns_2)
{
    InlineSourceLocation sl1("test", "some test string", 1, 1, 5, 8);
    InlineSourceLocation sl2("test", "some test string", 1, 1, 1, 2);
    sl1.unionize(&sl2);
    EXPECT_THAT(sl1.firstColumn(), Eq(1));
    EXPECT_THAT(sl1.lastColumn(), Eq(8));
}

TEST(NAME, unionize_lines_and_columns_1)
{
    InlineSourceLocation sl1("test", "some\ntest\nstring", 1, 1, 4, 5);
    InlineSourceLocation sl2("test", "some\ntest\nstring", 3, 3, 1, 2);
    sl1.unionize(&sl2);
    EXPECT_THAT(sl1.firstLine(), Eq(1));
    EXPECT_THAT(sl1.lastLine(), Eq(3));
    EXPECT_THAT(sl1.firstColumn(), Eq(4));
    EXPECT_THAT(sl1.lastColumn(), Eq(2));
}

TEST(NAME, unionize_lines_and_columns_2)
{
    InlineSourceLocation sl1("test", "some\ntest\nstring", 1, 1, 4, 5);
    InlineSourceLocation sl2("test", "some\ntest\nstring", 3, 3, 1, 2);
    sl2.unionize(&sl1);
    EXPECT_THAT(sl2.firstLine(), Eq(1));
    EXPECT_THAT(sl2.lastLine(), Eq(3));
    EXPECT_THAT(sl2.firstColumn(), Eq(4));
    EXPECT_THAT(sl2.lastColumn(), Eq(2));
}

TEST(NAME, unionize_lines_and_columns_3)
{
    InlineSourceLocation sl1("test", "some\ntest\nstring", 1, 1, 4, 5);
    InlineSourceLocation sl2("test", "some\ntest\nstring", 1, 1, 1, 2);
    sl1.unionize(&sl2);
    EXPECT_THAT(sl1.firstLine(), Eq(1));
    EXPECT_THAT(sl1.lastLine(), Eq(1));
    EXPECT_THAT(sl1.firstColumn(), Eq(1));
    EXPECT_THAT(sl1.lastColumn(), Eq(5));
}

TEST(NAME, unionize_lines_and_columns_4)
{
    InlineSourceLocation sl1("test", "some\ntest\nstring", 1, 1, 4, 5);
    InlineSourceLocation sl2("test", "some\ntest\nstring", 1, 1, 1, 2);
    sl2.unionize(&sl1);
    EXPECT_THAT(sl2.firstLine(), Eq(1));
    EXPECT_THAT(sl2.lastLine(), Eq(1));
    EXPECT_THAT(sl2.firstColumn(), Eq(1));
    EXPECT_THAT(sl2.lastColumn(), Eq(5));
}

TEST(NAME, unionize_lines_and_columns_5)
{
    InlineSourceLocation sl1("test", "some\ntest\nstring", 1, 1, 1, 2);
    InlineSourceLocation sl2("test", "some\ntest\nstring", 3, 3, 4, 5);
    sl1.unionize(&sl2);
    EXPECT_THAT(sl1.firstLine(), Eq(1));
    EXPECT_THAT(sl1.lastLine(), Eq(3));
    EXPECT_THAT(sl1.firstColumn(), Eq(1));
    EXPECT_THAT(sl1.lastColumn(), Eq(5));
}

TEST(NAME, unionize_lines_and_columns_6)
{
    InlineSourceLocation sl1("test", "some\ntest\nstring", 1, 1, 1, 2);
    InlineSourceLocation sl2("test", "some\ntest\nstring", 3, 3, 4, 5);
    sl2.unionize(&sl1);
    EXPECT_THAT(sl2.firstLine(), Eq(1));
    EXPECT_THAT(sl2.lastLine(), Eq(3));
    EXPECT_THAT(sl2.firstColumn(), Eq(1));
    EXPECT_THAT(sl2.lastColumn(), Eq(5));
}
