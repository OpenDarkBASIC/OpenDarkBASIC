#include "odb-util/tests/LogHelper.hpp"
#include <string>

#include <gmock/gmock.h>

extern "C" {
#include "odb-util/log.h"
#include "odb-util/utf8.h"
}

#define NAME odbutil_log

using namespace testing;

struct NAME : LogHelper, Test
{
    NAME() : LogHelper(2) {}
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
        LogEq("{emph}some/file.dba:2:8:{reset} "
              "{err}error:{reset} Assignment is bad for some "
              "reason\n"));
}

TEST_F(NAME, excerpt_1_one_sized_location)
{
    const char* source
        = "for a = 1 to 10\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct utf8_span loc = {23, 1};
    log_excerpt_1(source, loc, "test", 0);

    // clang-format off
    EXPECT_THAT(
        log(),
        LogEq(
            " 2 | if {emph0}a{reset} = 5 then a = 7\n"
            "   |    {emph0}^{reset} {emph0}test{reset}\n"));
    // clang-format on
}

TEST_F(NAME, excerpt_1_annotation_at_end)
{
    const char* source = "a = b + c\n";

    struct utf8_span loc = {8, 1};
    log_excerpt_1(source, loc, "test", 0);

    // clang-format off
    EXPECT_THAT(
        log(),
        LogEq(" 1 | a = b + {emph0}c{reset}\n"
              "   |         {emph0}^{reset} {emph0}test{reset}\n"));
    // clang-format on
}

TEST_F(NAME, excerpt_1_single_line)
{
    const char* source
        = "for a = 1 to 10\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct utf8_span loc = {23, 5}; /* a = 5 */
    log_excerpt_1(source, loc, "test", 0);

    // clang-format off
    EXPECT_THAT(
        log(),
        LogEq(" 2 | if {emph0}a = 5{reset} then a = 7\n"
              "   |    {emph0}^~~~<{reset} {emph0}test{reset}\n"));
    // clang-format on
}

TEST_F(NAME, excerpt_1_wrap_to_next_line)
{
    const char* source
        = "for a = 1 to 10\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct utf8_span loc = {34, 15}; /* a = 7 ... print */
    log_excerpt_1(source, loc, "test", 0);

    // clang-format off
    EXPECT_THAT(
        log(),
        LogEq(" 2 | if a = 5 then {emph0}a = 7{reset}\n"
              "   |               {emph0}^~~~~{reset} {emph0}test{reset}\n"
              " 3 | {emph0}print{reset} str$(a)\n"
              "   | {emph0}~~~~<{reset}\n"));
    // clang-format on
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
    log_excerpt_1(source, loc, "test", 0);

    EXPECT_THAT(
        log(),
        LogEq(" 2 |    {emph0}obj,{reset}\n"
              "   |    {emph0}^~~~{reset} {emph0}test{reset}\n"
              " 3 | {emph0}xpos,{reset}\n"
              "   | {emph0}~~~~~{reset}\n"
              " 4 | {emph0}      zpos{reset}\n"
              "   | {emph0}~~~~~~~~~<{reset}\n"));
}

TEST_F(NAME, excerpt_3_one_sized_locations_on_same_line)
{
    const char* source
        = "for a = 1 to 10\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct log_highlight inst[]
        = {{"", "test1", {23, 1}, LOG_HIGHLIGHT, LOG_MARKERS, 0},
           {"", "test2", {27, 1}, LOG_HIGHLIGHT, LOG_MARKERS, 0},
           {"", "test3", {29, 1}, LOG_HIGHLIGHT, LOG_MARKERS, 0},
           {0}};
    log_excerpt(source, inst);

    // clang-format off
    EXPECT_THAT(
        log(),
        LogEq(" 2 | if {emph0}a{reset} = {emph0}5{reset} {emph0}t{reset}hen a = 7\n"
              "   |    {emph0}^{reset}   {emph0}^{reset} {emph0}^{reset} {emph0}test3{reset}\n"
              "   |    {emph0}|{reset}   {emph0}test2{reset}\n"
              "   |    {emph0}test1{reset}\n"));
    // clang-format on
}

TEST_F(NAME, excerpt_3_wrap_to_next_line)
{
    const char* source
        = "for a = 1 to 10\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    // clang-format off
    struct log_highlight inst[]
        = {{"", "test1", {23, 5}, LOG_HIGHLIGHT, LOG_MARKERS, 0},  // a = 5
           {"", "test2", {34, 15}, LOG_HIGHLIGHT, LOG_MARKERS, 0}, // a = 7 .. print
           {"", "test3", {50, 4}, LOG_HIGHLIGHT, LOG_MARKERS, 0},  // str$
           {0}};
    log_excerpt(source, inst);

    EXPECT_THAT(
        log(),
        LogEq(" 2 | if {emph0}a = 5{reset} then {emph0}a = 7{reset}\n"
              "   |    {emph0}^~~~<{reset}      {emph0}^~~~~{reset} {emph0}test2{reset}\n"
              "   |    {emph0}test1{reset}\n"
              " 3 | {emph0}print{reset} {emph0}str${reset}(a)\n"
              "   | {emph0}~~~~<{reset} {emph0}^~~<{reset} {emph0}test3{reset}\n"));
    // clang-format on
}

TEST_F(NAME, excerpt_3_wrap_to_next_line_overlapping_annotations)
{
    const char* source
        = "for a = 1 to 10\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    /* clang-format off */
    struct log_highlight inst[] = {
        {"", "very long annotation", {23, 5}, LOG_HIGHLIGHT, LOG_MARKERS, 0},  // a = 1
        {"", "another long annotation", {29, 4}, LOG_HIGHLIGHT, LOG_MARKERS, 1}, // then
        {"", "test2", {34, 15}, LOG_HIGHLIGHT, LOG_MARKERS, 2}, // a = 7 .. print
        {"", "test3", {50, 4}, LOG_HIGHLIGHT, LOG_MARKERS, 3},  // str$
        LOG_HIGHLIGHT_SENTINAL
    };
    log_excerpt(source, inst);

    EXPECT_THAT(
        log(),
        LogEq(" 2 | if {emph0}a = 5{reset} {emph1}then{reset} {emph2}a = 7{reset}\n"
              "   |    {emph0}^~~~<{reset} {emph1}^~~<{reset} {emph2}^~~~~{reset} {emph2}test2{reset}\n"
              "   |    {emph0}|{reset}     {emph1}another long annotation{reset}\n"
              "   |    {emph0}very long annotation{reset}\n"
              " 3 | {emph2}print{reset} {emph0}str${reset}(a)\n"
              "   | {emph2}~~~~<{reset} {emph0}^~~<{reset} {emph0}test3{reset}\n"));
    /*clang-format on */
}

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

    // clang-format off
    EXPECT_THAT(
        log(),
        LogEq(" 2 | variable = {emph0}v{reset}ariable {emph2}and{reset} {emph1}333{reset}\n"
              "   |            {emph0}^{reset}        {emph2}^^^{reset} {emph1}~~<{reset} {emph1}constant{reset}\n"
              "   |            {emph0}variable{reset}\n"));
    // clang-format on
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

    // clang-format off
    EXPECT_THAT(
        log(),
        LogEq(" 2 | variable = {emph0}variable{reset} {emph2}a{reset}nd {emph1}333{reset}\n"
              "   |            {emph0}>~~~~~~~{reset} {emph2}^{reset}   {emph1}~~<{reset} {emph1}constant{reset}\n"
              "   |            {emph0}variable{reset}\n"));
    // clang-format on
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

    // clang-format off
    EXPECT_THAT(
        log(),
        LogEq(" 2 | variable = {emph0}variable{reset} {emph2}and{reset} {emph1}3{reset}33\n"
              "   |            {emph0}>~~~~~~~{reset} {emph2}^^^{reset} {emph1}^{reset} {emph1}constant{reset}\n"
              "   |            {emph0}variable{reset}\n"));
    // clang-format on
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

    // clang-format off
    EXPECT_THAT(
        log(),
        LogEq(" 2 | variable = {emph0}variable{reset} {emph2}and{reset} {emph1}333{reset}\n"
              "   |            {emph0}>~~~~~~~{reset} {emph2}^^^{reset} {emph1}~~<{reset} {emph1}constant{reset}\n"
              "   |            {emph0}variable{reset}\n"));
    // clang-format on
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
        LogEq(" 2 | variable = {emph0}variable{reset}\n"
              "   |            {emph0}>~~~~~~~{reset} {emph0}variable{reset}\n"
              " 3 |             {emph2}and{reset} {emph1}333{reset}\n"
              "   |             {emph2}^^^{reset} {emph1}~~<{reset} "
              "{emph1}constant{reset}\n"));
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
        LogEq(" 2 | variable = {emph0}variable{reset} {emph2}and{reset}\n"
              "   |            {emph0}>~~~~~~~{reset} {emph2}^^^{reset}\n"
              "   |            {emph0}variable{reset}\n"
              " 3 |             {emph1}333{reset}\n"
              "   |             {emph1}~~<{reset} {emph1}constant{reset}\n"));
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

    // clang-format off
    EXPECT_THAT(
        log(),
        LogEq(" 2 | variable = {emph0}variable{reset}\n"
              "   |            {emph0}>~~~~~~~{reset} {emph0}variable{reset}\n"
              " 3 |               {emph2}and{reset}\n"
              "   |               {emph2}^^^{reset}\n"
              " 4 |            {emph1}333{reset}\n"
              "   |            {emph1}~~<{reset} {emph1}constant{reset}\n"));
    // clang-format on
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
        LogEq(" 2 | variable = {emph0}some_func({reset}\n"
              "   |            {emph0}>~~~~~~~~~{reset} {emph0}first "
              "operand{reset}\n"
              " 3 | {emph0}    arg1,{reset}\n"
              "   | {emph0}~~~~~~~~~{reset}\n"
              " 4 | {emph0}    arg2){reset}\n"
              "   | {emph0}~~~~~~~~~{reset}\n"
              " 5 |               {emph2}and{reset}\n"
              "   |               {emph2}^^^{reset}\n"
              " 6 |        {emph1}another_func({reset}\n"
              "   |        {emph1}~~~~~~~~~~~~~{reset} {emph1}second "
              "operand{reset}\n"
              " 7 | {emph1}         23,{reset}\n"
              "   | {emph1}~~~~~~~~~~~~{reset}\n"
              " 8 | {emph1}           333){reset}\n"
              "   | {emph1}~~~~~~~~~~~~~~<{reset}\n"));
}

TEST_F(NAME, insert_excerpt1)
{
    const char* source
        = "for a = 1 to 10\n"
          "    if a - 2\n"
          "    print str$(a)\n"
          "next a\n";

    struct log_highlight inst[]
        = {{"(", "", {23, 1}, LOG_INSERT, LOG_MARKERS, 0},
           {") <> 0", "", {28, 6}, LOG_INSERT, LOG_MARKERS, 0},
           {0}};
    log_excerpt(source, inst);

    // clang-format off
    EXPECT_THAT(
        log(),
        LogEq(" 2 | if {insert}({reset}a - 2{insert}) <> 0{reset}\n"
              "   |    {insert}^{reset}     {insert}^~~~~<{reset}\n"));
    // clang-format on
}

TEST_F(NAME, insert_excerpt2)
{
    const char* source
        = "for a = 1 to 10\n"
          "    if a - 2 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct log_highlight inst[]
        = {{"(", "", {23, 1}, LOG_INSERT, LOG_MARKERS, 0},
           {") <> 0", "", {28, 6}, LOG_INSERT, LOG_MARKERS, 0},
           {0}};
    log_excerpt(source, inst);

    // clang-format off
    EXPECT_THAT(
        log(),
        LogEq(" 2 | if {insert}({reset}a - 2{insert}) <> 0{reset} then a = 7\n"
              "   |    {insert}^{reset}     {insert}^~~~~<{reset}\n"));
    // clang-format on
}

TEST_F(NAME, insert_excerpt3)
{
    const char* source
        = "for a = 1 to 10\n"
          "    if a - 2 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct log_highlight inst[]
        = {{"(0 + ", "", {23, 5}, LOG_INSERT, LOG_MARKERS, 0},
           {") <> 0", "", {28, 6}, LOG_INSERT, LOG_MARKERS, 0},
           {0}};
    log_excerpt(source, inst);

    // clang-format off
    EXPECT_THAT(
        log(),
        LogEq(" 2 | if {insert}(0 + {reset}a - 2{insert}) <> 0{reset} then a = 7\n"
              "   |    {insert}^~~~<{reset}     {insert}^~~~~<{reset}\n"));
    // clang-format on
}

TEST_F(NAME, insert_and_highlight_excerpt)
{
    const char* source
        = "for a = 1 to 10\n"
          "    if a - 2 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct log_highlight inst[]
        = {{"(", "", {23, 1}, LOG_INSERT, LOG_MARKERS, 1},
           {"", "test1", {23, 5}, LOG_HIGHLIGHT, LOG_MARKERS, 0},
           {") <> 0", "test2", {28, 6}, LOG_INSERT, LOG_MARKERS, 1},
           {0}};
    log_excerpt(source, inst);

    // clang-format off
    EXPECT_THAT(
        log(),
        LogEq(" 2 | if {insert}({reset}{emph0}a - 2{reset}{insert}) <> 0{reset} then a = 7\n"
              "   |    {insert}^{reset}{emph0}^~~~<{reset}{insert}^~~~~<{reset} {emph1}test2{reset}\n"
              "   |    {emph0}test1{reset}\n"));
    // clang-format on
}

TEST_F(NAME, issue)
{
    const char* source
        = "for n=a to b\n"
          "next\n";

    const struct log_highlight inst[]
        = {{" STEP 1", "", {12, 7}, LOG_INSERT, LOG_MARKERS, 0}, {0}};
    log_excerpt(source, inst);
    EXPECT_THAT(
        log(),
        LogEq(" 1 | for n=a to b{insert} STEP 1{reset}\n"
              "   |             {insert}^~~~~~<{reset}\n"));
}

TEST_F(NAME, insert_at_end_without_newline)
{
    const char* source = "print a and b";

    const struct log_highlight hl[]
        = {{" <> 0", "", {13, 5}, LOG_INSERT, LOG_MARKERS, 0},
           LOG_HIGHLIGHT_SENTINAL};
    log_excerpt(source, hl);
    EXPECT_THAT(
        log(),
        LogEq(" 1 | print a and b{insert} <> 0{reset}\n"
              "   |              {insert}^~~~<{reset}\n"));
}
