#include "gmock/gmock.h"
#include "odb-compiler/ast/SourceLocation.hpp"

#define NAME SourceLocation

using namespace testing;
using namespace odb::ast;

TEST(NAME, EmptyLocation)
{
    InlineSourceLocation sl("test", "", 1, 1, 1, 2);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:1"));
}

TEST(NAME, SingleCharLocation)
{
    InlineSourceLocation sl("test", "some command 1, 2, 3", 1, 1, 2, 3);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:2"));

    std::vector<std::string> sh = sl.getSectionHighlight();
    EXPECT_THAT(sh[0], StrEq("some command 1, 2, 3"));
    EXPECT_THAT(sh[1], StrEq(" ^"));
}

TEST(NAME, MultiCharLocation1)
{
    InlineSourceLocation sl("test", "some command 1, 2, 3", 1, 1, 4, 8);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:4"));

    std::vector<std::string> sh = sl.getSectionHighlight();
    EXPECT_THAT(sh[0], StrEq("some command 1, 2, 3"));
    EXPECT_THAT(sh[1], StrEq("   ^~~~"));
}

TEST(NAME, MultiCharLocation2)
{
    InlineSourceLocation sl("test", "some command 1, 2, 3\nanother command 4, 5, 6\n", 2, 2, 4, 8);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:2:4"));

    std::vector<std::string> sh = sl.getSectionHighlight();
    EXPECT_THAT(sh[0], StrEq("another command 4, 5, 6"));
    EXPECT_THAT(sh[1], StrEq("   ^~~~"));
}

TEST(NAME, MultiLineLocation1)
{
    InlineSourceLocation sl("test", "some command 1, 2, 3\nanother command 4, 5, 6\n", 1, 2, 4, 8);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:4"));

    std::vector<std::string> sh = sl.getSectionHighlight();
    EXPECT_THAT(sh[0], StrEq("some command 1, 2, 3"));
    EXPECT_THAT(sh[1], StrEq("   ^~~~~~~~~~~~~~~~~"));
    EXPECT_THAT(sh[2], StrEq("another command 4, 5, 6"));
    EXPECT_THAT(sh[3], StrEq("~~~~~~~"));
}

TEST(NAME, MultiLineLocation2)
{
    InlineSourceLocation sl("test", "some command 1, 2, 3\nanother command 4, 5, 6\n", 1, 2, 9, 8);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:9"));

    std::vector<std::string> sh = sl.getSectionHighlight();
    EXPECT_THAT(sh[0], StrEq("some command 1, 2, 3"));
    EXPECT_THAT(sh[1], StrEq("        ^~~~~~~~~~~~"));
    EXPECT_THAT(sh[2], StrEq("another command 4, 5, 6"));
    EXPECT_THAT(sh[3], StrEq("~~~~~~~"));
}

TEST(NAME, MultiLineLocation3)
{
    InlineSourceLocation sl("test", "some command 1, 2, 3\nanother command 4, 5, 6\nyet another command 7, 8, 9",
                            1, 3, 4, 8);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:4"));

    std::vector<std::string> sh = sl.getSectionHighlight();
    EXPECT_THAT(sh[0], StrEq("some command 1, 2, 3"));
    EXPECT_THAT(sh[1], StrEq("   ^~~~~~~~~~~~~~~~~"));
    EXPECT_THAT(sh[2], StrEq("another command 4, 5, 6"));
    EXPECT_THAT(sh[3], StrEq("~~~~~~~~~~~~~~~~~~~~~~~"));
    EXPECT_THAT(sh[4], StrEq("yet another command 7, 8, 9"));
    EXPECT_THAT(sh[5], StrEq("~~~~~~~"));
}

TEST(NAME, MultiLineLocation4)
{
    InlineSourceLocation sl("test", "some command 1, 2, 3\nanother command 4, 5, 6\nyet another command 7, 8, 9",
                            1, 3, 7, 5);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:7"));

    std::vector<std::string> sh = sl.getSectionHighlight();
    EXPECT_THAT(sh[0], StrEq("some command 1, 2, 3"));
    EXPECT_THAT(sh[1], StrEq("      ^~~~~~~~~~~~~~"));
    EXPECT_THAT(sh[2], StrEq("another command 4, 5, 6"));
    EXPECT_THAT(sh[3], StrEq("~~~~~~~~~~~~~~~~~~~~~~~"));
    EXPECT_THAT(sh[4], StrEq("yet another command 7, 8, 9"));
    EXPECT_THAT(sh[5], StrEq("~~~~"));
}

TEST(NAME, TabsNSpaces1)
{
    InlineSourceLocation sl("test", "  \t    some command 1, 2, 3", 1, 1, 8, 12);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:8"));

    std::vector<std::string> sh = sl.getSectionHighlight();
    EXPECT_THAT(sh[0], StrEq("          some command 1, 2, 3"));
    EXPECT_THAT(sh[1], StrEq("          ^~~~"));
}

TEST(NAME, TabsNSpaces2)
{
    InlineSourceLocation sl("test", "      \tsome command 1, 2, 3", 1, 1, 8, 12);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:8"));

    std::vector<std::string> sh = sl.getSectionHighlight();
    EXPECT_THAT(sh[0], StrEq("          some command 1, 2, 3"));
    EXPECT_THAT(sh[1], StrEq("          ^~~~"));
}

TEST(NAME, TabsNSpaces3)
{
    InlineSourceLocation sl("test", "  \t    some\tcommand 1, 2, 3", 1, 1, 7, 14);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:7"));

    std::vector<std::string> sh = sl.getSectionHighlight();
    EXPECT_THAT(sh[0], StrEq("          some    command 1, 2, 3"));
    EXPECT_THAT(sh[1], StrEq("         ^~~~~~~~~~"));
}

TEST(NAME, TabsNSpaces4)
{
    InlineSourceLocation sl("test", "\t  some command 1, 2, 3\n\tanother command 4, 5, 6\n\t\tyet another command 7, 8, 9",
                            1, 3, 9, 6);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:9"));

    std::vector<std::string> sh = sl.getSectionHighlight();
    EXPECT_THAT(sh[0], StrEq("      some command 1, 2, 3"));
    EXPECT_THAT(sh[1], StrEq("           ^~~~~~~~~~~~~~~"));
    EXPECT_THAT(sh[2], StrEq("    another command 4, 5, 6"));
    EXPECT_THAT(sh[3], StrEq("    ~~~~~~~~~~~~~~~~~~~~~~~"));
    EXPECT_THAT(sh[4], StrEq("        yet another command 7, 8, 9"));
    EXPECT_THAT(sh[5], StrEq("        ~~~"));
}

TEST(NAME, TabsNSpaces5)
{
    InlineSourceLocation sl("test", "  \tsome command 1, 2, 3\n\tanother\tcommand 4, 5, 6\n\t\tyet\tanother command 7, 8, 9",
                            1, 3, 3, 6);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:3"));

    std::vector<std::string> sh = sl.getSectionHighlight();
    EXPECT_THAT(sh[0], StrEq("      some command 1, 2, 3"));
    EXPECT_THAT(sh[1], StrEq("  ^~~~~~~~~~~~~~~~~~~~~~~~"));
    EXPECT_THAT(sh[2], StrEq("    another    command 4, 5, 6"));
    EXPECT_THAT(sh[3], StrEq("    ~~~~~~~~~~~~~~~~~~~~~~~~~~"));
    EXPECT_THAT(sh[4], StrEq("        yet    another command 7, 8, 9"));
    EXPECT_THAT(sh[5], StrEq("        ~~~"));
}

TEST(NAME, TabsNSpaces6)
{
    InlineSourceLocation sl("test", "  \tsome command 1, 2, 3\n\tanother\tcommand 4, 5, 6\t\n\t\tyet\tanother command 7, 8, 9",
                            1, 3, 4, 6);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:4"));

    std::vector<std::string> sh = sl.getSectionHighlight();
    EXPECT_THAT(sh[0], StrEq("      some command 1, 2, 3"));
    EXPECT_THAT(sh[1], StrEq("      ^~~~~~~~~~~~~~~~~~~~"));
    EXPECT_THAT(sh[2], StrEq("    another    command 4, 5, 6    "));
    EXPECT_THAT(sh[3], StrEq("    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
    EXPECT_THAT(sh[4], StrEq("        yet    another command 7, 8, 9"));
    EXPECT_THAT(sh[5], StrEq("        ~~~"));
}

TEST(NAME, InvalidLocation1)
{
    InlineSourceLocation sl("test", "lol", 1, 1, 4, 5);
    std::vector<std::string> sh = sl.getSectionHighlight();
    EXPECT_THAT(sh[0], StrEq("(Invalid location 1,1,4,5)"));
}

TEST(NAME, InvalidLocation2)
{
    InlineSourceLocation sl("test", "lol", 1, 1, 3, 5);
    std::vector<std::string> sh = sl.getSectionHighlight();
    EXPECT_THAT(sh[0], StrEq("(Invalid location 1,1,3,5)"));
}

TEST(NAME, InvalidLocation3)
{
    InlineSourceLocation sl("test", "lol", 2, 2, 2, 3);
    std::vector<std::string> sh = sl.getSectionHighlight();
    EXPECT_THAT(sh[0], StrEq("(Invalid location 2,2,2,3)"));
}

TEST(NAME, InvalidLocation4)
{
    InlineSourceLocation sl("test", "lol", 1, 2, 2, 3);
    std::vector<std::string> sh = sl.getSectionHighlight();
    EXPECT_THAT(sh[0], StrEq("(Invalid location 1,2,2,3)"));
}

TEST(NAME, InvalidLocation5)
{
    InlineSourceLocation sl("test", "lol\nlel\nlil", 1, 3, 4, 3);
    std::vector<std::string> sh = sl.getSectionHighlight();
    EXPECT_THAT(sh[0], StrEq("(Invalid location 1,3,4,3)"));
}

TEST(NAME, InvalidLocation6)
{
    InlineSourceLocation sl("test", "lol\nlel\nlil", 1, 3, 2, 5);
    std::vector<std::string> sh = sl.getSectionHighlight();
    EXPECT_THAT(sh[0], StrEq("(Invalid location 1,3,2,5)"));
}

TEST(NAME, InvalidLocation7)
{
    InlineSourceLocation sl("test", "lol\nlel\nlil", 1, 3, 2, 2);
    std::vector<std::string> sh = sl.getSectionHighlight();
    EXPECT_THAT(sh[0], StrEq("(Invalid location 1,3,2,2)"));
}

TEST(NAME, EmptyLines)
{
    InlineSourceLocation sl("test", "some command 1, 2, 3\n\n\nanother command 4, 5, 6\nyet another command 7, 8, 9",
                            1, 5, 4, 8);
    EXPECT_THAT(sl.getFileLineColumn(), StrEq("test:1:4"));

    std::vector<std::string> sh = sl.getSectionHighlight();
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

TEST(NAME, join_columns_1)
{
    InlineSourceLocation sl1("test", "some test string", 1, 1, 5, 8);
    InlineSourceLocation sl2("test", "some test string", 1, 1, 6, 10);
    sl1.join(&sl2);
    EXPECT_THAT(sl1.getFirstColumn(), Eq(5));
    EXPECT_THAT(sl1.getLastColumn(), Eq(10));
}

TEST(NAME, join_columns_2)
{
    InlineSourceLocation sl1("test", "some test string", 1, 1, 5, 8);
    InlineSourceLocation sl2("test", "some test string", 1, 1, 1, 2);
    sl1.join(&sl2);
    EXPECT_THAT(sl1.getFirstColumn(), Eq(1));
    EXPECT_THAT(sl1.getLastColumn(), Eq(8));
}

TEST(NAME, join_lines_and_columns_1)
{
    InlineSourceLocation sl1("test", "some\ntest\nstring", 1, 1, 4, 5);
    InlineSourceLocation sl2("test", "some\ntest\nstring", 3, 3, 1, 2);
    sl1.join(&sl2);
    EXPECT_THAT(sl1.getFirstLine(), Eq(1));
    EXPECT_THAT(sl1.getLastLine(), Eq(3));
    EXPECT_THAT(sl1.getFirstColumn(), Eq(4));
    EXPECT_THAT(sl1.getLastColumn(), Eq(2));
}

TEST(NAME, join_lines_and_columns_2)
{
    InlineSourceLocation sl1("test", "some\ntest\nstring", 1, 1, 4, 5);
    InlineSourceLocation sl2("test", "some\ntest\nstring", 1, 1, 1, 2);
    sl1.join(&sl2);
    EXPECT_THAT(sl1.getFirstLine(), Eq(1));
    EXPECT_THAT(sl1.getLastLine(), Eq(1));
    EXPECT_THAT(sl1.getFirstColumn(), Eq(1));
    EXPECT_THAT(sl1.getLastColumn(), Eq(5));
}

TEST(NAME, join_lines_and_columns_3)
{
    InlineSourceLocation sl1("test", "some\ntest\nstring", 1, 1, 1, 2);
    InlineSourceLocation sl2("test", "some\ntest\nstring", 3, 3, 4, 5);
    sl1.join(&sl2);
    EXPECT_THAT(sl1.getFirstLine(), Eq(1));
    EXPECT_THAT(sl1.getLastLine(), Eq(3));
    EXPECT_THAT(sl1.getFirstColumn(), Eq(1));
    EXPECT_THAT(sl1.getLastColumn(), Eq(5));
}
