#include "odb-sdk/tests/LogHelper.hpp"
#include <string>

#include <gmock/gmock.h>

extern "C" {
#include "odb-sdk/log.h"
#include "odb-sdk/utf8.h"
}

#define NAME odbsdk_log

using namespace testing;

struct NAME : LogHelper, Test
{
};

TEST_F(NAME, file_line_column)
{
    const char* source
        = "for a = 1 to 10\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct utf8_span loc = {23, 5}; /* a = 5 */
    log_flc_err(
        "some/file.dba",
        source,
        loc,
        "Assignment is bad %s\n",
        "for some reason");

    EXPECT_THAT(
        log(),
        LogEq("some/file.dba:2:8: error: Assignment is bad for some reason\n"));
}

TEST_F(NAME, excerpt_1_one_sized_location)
{
    const char* source
        = "for a = 1 to 10\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct utf8_span loc = {23, 1};
    log_excerpt_1(source, loc, "test");

    EXPECT_THAT(
        log(),
        LogEq(" 2 | if a = 5 then a = 7\n"
              "   |    ^ test\n"));
}

TEST_F(NAME, excerpt_1_annotation_at_end)
{
    const char* source = "a = b + c\n";

    struct utf8_span loc = {8, 1};
    log_excerpt_1(source, loc, "test");

    EXPECT_THAT(
        log(),
        LogEq(" 1 | a = b + c\n"
              "   |         ^ test\n"));
}

TEST_F(NAME, excerpt_1_single_line)
{
    const char* source
        = "for a = 1 to 10\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct utf8_span loc = {23, 5}; /* a = 5 */
    log_excerpt_1(source, loc, "test");

    EXPECT_THAT(
        log(),
        LogEq(" 2 | if a = 5 then a = 7\n"
              "   |    ^~~~< test\n"));
}

TEST_F(NAME, excerpt_1_wrap_to_next_line)
{
    const char* source
        = "for a = 1 to 10\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct utf8_span loc = {34, 15}; /* a = 7 ... print */
    log_excerpt_1(source, loc, "test");

    EXPECT_THAT(
        log(),
        LogEq(" 2 | if a = 5 then a = 7\n"
              "   |               ^~~~~\n"
              "   |               test\n"
              " 3 | print str$(a)\n"
              "   | ~~~~<\n"));
}

TEST_F(NAME, excerpt_1_multiple_lines)
{
    const char* source
        = "a = get ground height(\n"
          "       obj,\n"
          "    xpos,\n"
          "          zpos\n"
          ")\n";

    struct utf8_span loc = {30, 29}; /* 3 arguments */
    log_excerpt_1(source, loc, "test");

    EXPECT_THAT(
        log(),
        LogEq(" 2 |    obj,\n"
              "   |    ^~~~\n"
              "   |    test\n"
              " 3 | xpos,\n"
              "   | ~~~~~\n"
              " 4 |       zpos\n"
              "   | ~~~~~~~~~<\n"));
}

TEST_F(NAME, excerpt_3_one_sized_locations_on_same_line)
{
    const char* source
        = "for a = 1 to 10\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct log_excerpt_inst inst[]
        = {{"", "test1", {23, 1}, LOG_EXCERPT_HIGHLIGHT, 0},
           {"", "test2", {27, 1}, LOG_EXCERPT_HIGHLIGHT, 0},
           {"", "test3", {29, 1}, LOG_EXCERPT_HIGHLIGHT, 0},
           {0}};
    log_excerpt(source, inst);

    EXPECT_THAT(
        log(),
        LogEq(" 2 | if a = 5 then a = 7\n"
              "   |    ^   ^ ^ test3\n"
              "   |    |   test2\n"
              "   |    test1\n"));
}

#if 0
TEST_F(NAME, excerpt_3_annotation_at_end)
{
    const char* source = "a = b + c\n";

    struct utf8_span loc = {8, 1};
    log_excerpt_3(source, loc, "test");

    EXPECT_THAT(
        log(),
        LogEq(" 1 | a = b + c\n"
              "   |         ^ test\n"));
}

TEST_F(NAME, excerpt_3_single_line)
{
    const char* source
        = "for a = 1 to 10\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct utf8_span loc = {23, 5}; /* a = 5 */
    log_excerpt_3(source, loc, "test");

    EXPECT_THAT(
        log(),
        LogEq(" 2 | if a = 5 then a = 7\n"
              "   |    ^~~~< test\n"));
}
#endif

TEST_F(NAME, excerpt_3_wrap_to_next_line)
{
    const char* source
        = "for a = 1 to 10\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct log_excerpt_inst inst[]
        = {{"", "test1", {23, 5}, LOG_EXCERPT_HIGHLIGHT, 0},  // a = 1
           {"", "test2", {34, 15}, LOG_EXCERPT_HIGHLIGHT, 0}, // a = 7 .. print
           {"", "test3", {50, 4}, LOG_EXCERPT_HIGHLIGHT, 0},  // str$
           {0}};
    log_excerpt(source, inst);

    EXPECT_THAT(
        log(),
        LogEq(" 2 | if a = 5 then a = 7\n"
              "   |    ^~~~<      ^~~~~\n"
              "   |    |          test2\n"
              "   |    test1\n"
              " 3 | print str$(a)\n"
              "   | ~~~~< ^~~< test3\n"));
}
TEST_F(NAME, excerpt_3_wrap_to_next_line_overlapping_annotations)
{
    const char* source
        = "for a = 1 to 10\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    /* clang-format off */
    struct log_excerpt_inst inst[]
        = {{"", "very long annotation", {23, 5}, LOG_EXCERPT_HIGHLIGHT, 0},  // a = 1
           {"", "test2", {34, 15}, LOG_EXCERPT_HIGHLIGHT, 0}, // a = 7 .. print
           {"", "test3", {50, 4}, LOG_EXCERPT_HIGHLIGHT, 0},  // str$
           {0}};
    log_excerpt(source, inst);
    /*clang-format on */

    EXPECT_THAT(
        log(),
        LogEq(" 2 | if a = 5 then a = 7\n"
              "   |    ^~~~<      ^~~~~\n"
              "   |    |          test2\n"
              "   |    very long annotation\n"
              " 3 | print str$(a)\n"
              "   | ~~~~< ^~~< test3\n"));
}

#if 0
TEST_F(NAME, excerpt_3_multiple_lines)
{
    const char* source
        = "a = get ground height(\n"
          "       obj,\n"
          "    xpos,\n"
          "          zpos\n"
          ")\n";

    struct utf8_span loc = {30, 29}; /* 3 arguments */
    log_excerpt_3(source, loc, "test");

    EXPECT_THAT(
        log(),
        LogEq(" 2 |    obj,\n"
              "   |    ^~~~\n"
              " 3 | xpos,\n"
              "   | ~~~~~\n"
              " 4 |       zpos\n"
              "   | ~~~~~~~~~< test\n"));
}
#endif

TEST_F(NAME, excerpt_binop_one_sized_lhs_location)
{
    const char* source
        = "for a = 1 to 10\n"
          "    variable = variable and 333\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct utf8_span lhs = {31, 1};
    struct utf8_span op = {40, 3};
    struct utf8_span rhs = {44, 3};
    log_excerpt_binop(source, lhs, op, rhs, "variable", "constant");

    EXPECT_THAT(
        log(),
        LogEq(" 2 | variable = variable and 333\n"
              "   |            ^        ^^^ ~~< constant\n"
              "   |            variable\n"));
}

TEST_F(NAME, excerpt_binop_one_sized_op_location)
{
    const char* source
        = "for a = 1 to 10\n"
          "    variable = variable and 333\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct utf8_span lhs = {31, 8};
    struct utf8_span op = {40, 1};
    struct utf8_span rhs = {44, 3};
    log_excerpt_binop(source, lhs, op, rhs, "variable", "constant");

    EXPECT_THAT(
        log(),
        LogEq(" 2 | variable = variable and 333\n"
              "   |            >~~~~~~~ ^   ~~< constant\n"
              "   |            variable\n"));
}

TEST_F(NAME, excerpt_binop_one_sized_rhs_location)
{
    const char* source
        = "for a = 1 to 10\n"
          "    variable = variable and 333\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct utf8_span lhs = {31, 8};
    struct utf8_span op = {40, 3};
    struct utf8_span rhs = {44, 1};
    log_excerpt_binop(source, lhs, op, rhs, "variable", "constant");

    EXPECT_THAT(
        log(),
        LogEq(" 2 | variable = variable and 333\n"
              "   |            >~~~~~~~ ^^^ ^ constant\n"
              "   |            variable\n"));
}

TEST_F(NAME, excerpt_binop_single_line)
{
    const char* source
        = "for a = 1 to 10\n"
          "    variable = variable and 333\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct utf8_span lhs = {31, 8};
    struct utf8_span op = {40, 3};
    struct utf8_span rhs = {44, 3};
    log_excerpt_binop(source, lhs, op, rhs, "variable", "constant");

    EXPECT_THAT(
        log(),
        LogEq(" 2 | variable = variable and 333\n"
              "   |            >~~~~~~~ ^^^ ~~< constant\n"
              "   |            variable\n"));
}

TEST_F(NAME, excerpt_binop_wrap_to_next_line1)
{
    const char* source
        = "for a = 1 to 10\n"
          "    variable = variable\n"
          "                and 333\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct utf8_span lhs = {31, 8};
    struct utf8_span op = {56, 3};
    struct utf8_span rhs = {60, 3};
    log_excerpt_binop(source, lhs, op, rhs, "variable", "constant");

    EXPECT_THAT(
        log(),
        LogEq(" 2 | variable = variable\n"
              "   |            >~~~~~~~ variable\n"
              " 3 |             and 333\n"
              "   |             ^^^ ~~< constant\n"));
}

TEST_F(NAME, excerpt_binop_wrap_to_next_line2)
{
    const char* source
        = "for a = 1 to 10\n"
          "    variable = variable and\n"
          "                333\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct utf8_span lhs = {31, 8};
    struct utf8_span op = {40, 3};
    struct utf8_span rhs = {60, 3};
    log_excerpt_binop(source, lhs, op, rhs, "variable", "constant");

    EXPECT_THAT(
        log(),
        LogEq(" 2 | variable = variable and\n"
              "   |            >~~~~~~~ ^^^ variable\n"
              " 3 |             333\n"
              "   |             ~~< constant\n"));
}

TEST_F(NAME, excerpt_binop_wrap_to_next_line3)
{
    const char* source
        = "for a = 1 to 10\n"
          "    variable = variable\n"
          "                  and\n"
          "               333\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct utf8_span lhs = {31, 8};
    struct utf8_span op = {58, 3};
    struct utf8_span rhs = {77, 3};
    log_excerpt_binop(source, lhs, op, rhs, "variable", "constant");

    EXPECT_THAT(
        log(),
        LogEq(" 2 | variable = variable\n"
              "   |            >~~~~~~~ variable\n"
              " 3 |               and\n"
              "   |               ^^^\n"
              " 4 |            333\n"
              "   |            ~~< constant\n"));
}

TEST_F(NAME, excerpt_binop_everything_wraps)
{
    const char* source
        = "for a = 1 to 10\n"
          "    variable = some_func(\n"
          "        arg1,\n"
          "        arg2)\n"
          "                  and\n"
          "           another_func(\n"
          "             23,\n"
          "               333)\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct utf8_span lhs = {31, 38};
    struct utf8_span op = {88, 3};
    struct utf8_span rhs = {103, 50};
    log_excerpt_binop(source, lhs, op, rhs, "first operand", "second operand");

    EXPECT_THAT(
        log(),
        LogEq(" 2 | variable = some_func(\n"
              "   |            >~~~~~~~~~\n"
              " 3 |     arg1,\n"
              "   | ~~~~~~~~~\n"
              " 4 |     arg2)\n"
              "   | ~~~~~~~~~ first operand\n"
              " 5 |               and\n"
              "   |               ^^^\n"
              " 6 |        another_func(\n"
              "   |        ~~~~~~~~~~~~~\n"
              " 7 |          23,\n"
              "   | ~~~~~~~~~~~~\n"
              " 8 |            333)\n"
              "   | ~~~~~~~~~~~~~~< second operand\n"));
}

TEST_F(NAME, insert_excerpt1)
{
    const char* source
        = "for a = 1 to 10\n"
          "    if a - 2\n"
          "    print str$(a)\n"
          "next a\n";

    struct log_excerpt_inst inst[]
        = {{"(", "", {23, 1}, LOG_EXCERPT_INSERT, 0},
           {") <> 0", "", {28, 6}, LOG_EXCERPT_INSERT, 0},
           {0}};
    log_excerpt(source, inst);
    EXPECT_THAT(
        log(),
        LogEq(" 2 | if (a - 2) <> 0\n"
              "   |    ^     ^~~~~<\n"));
}


TEST_F(NAME, insert_excerpt2)
{
    const char* source
        = "for a = 1 to 10\n"
          "    if a - 2 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct log_excerpt_inst inst[]
        = {{"(", "", {23, 1}, LOG_EXCERPT_INSERT, 0},
           {") <> 0", "", {28, 6}, LOG_EXCERPT_INSERT, 0},
           {0}};
    log_excerpt(source, inst);
    EXPECT_THAT(
        log(),
        LogEq(" 2 | if (a - 2) <> 0 then a = 7\n"
              "   |    ^     ^~~~~<\n"));
}

TEST_F(NAME, insert_excerpt3)
{
    const char* source
        = "for a = 1 to 10\n"
          "    if a - 2 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct log_excerpt_inst inst[]
        = {{"(0 + ", "", {23, 5}, LOG_EXCERPT_INSERT, 0},
           {") <> 0", "", {28, 6}, LOG_EXCERPT_INSERT, 0},
           {0}};
    log_excerpt(source, inst);
    EXPECT_THAT(
        log(),
        LogEq(" 2 | if (0 + a - 2) <> 0 then a = 7\n"
              "   |    ^~~~<     ^~~~~<\n"));
}
