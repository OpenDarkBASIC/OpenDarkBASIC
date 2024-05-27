#include <string>

#include <gmock/gmock.h>

extern "C" {
#include "odb-sdk/log.h"
#include "odb-sdk/utf8.h"
}

#define NAME odbsdk_log

using namespace testing;

struct LogOutput
{
    std::string text;
};

struct LogEqMatcher : testing::MatcherInterface<const LogOutput&>
{
    explicit LogEqMatcher(const char* expected) : expected(expected) {}

    bool
    MatchAndExplain(
        const LogOutput&              logOutput,
        testing::MatchResultListener* listener) const override
    {
        *listener << "Log:\n" << logOutput.text;
        return logOutput.text == expected;
    }
    void
    DescribeTo(::std::ostream* os) const override
    {
        *os << "Log output equals:\n" << expected;
    }
    void
    DescribeNegationTo(::std::ostream* os) const override
    {
        *os << "Log output does not equal:\n" << expected;
    }

    std::string expected;
};

inline testing::Matcher<const LogOutput&>
LogEq(const char* expected)
{
    return testing::MakeMatcher(new LogEqMatcher(expected));
}

static LogOutput log_output;
static void
write_str(const char* fmt, va_list ap)
{
    va_list ap2;
    va_copy(ap2, ap);
    int len = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);

    size_t off = log_output.text.size();
    log_output.text.resize(log_output.text.size() + len + 1);
    vsprintf(log_output.text.data() + off, fmt, ap);
    log_output.text.resize(log_output.text.size() - 1);
}

struct NAME : public Test
{
    void
    SetUp() override
    {
        struct log_interface i = {write_str, 0};
        old_log_interface = log_configure(i);
    }

    void
    TearDown() override
    {
        log_configure(old_log_interface);
        log_output.text.clear();
    }

    struct log_interface old_log_interface;
};

TEST_F(NAME, test)
{
    log_sdk_note("Hello\n");
}

TEST_F(NAME, file_line_column)
{
    const char* source
        = "for a = 1 to 10\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct utf8_span loc = {23, 5}; /* a = 5 */
    log_flc(
        "{e:error: }",
        "some/file.dba",
        source,
        loc,
        "Assignment is bad %s\n",
        "for some reason");

    EXPECT_THAT(
        log_output,
        LogEq("some/file.dba:2:8: error: Assignment is bad for some reason\n"));
}

TEST_F(NAME, excerpt_single_line)
{
    const char* source
        = "for a = 1 to 10\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct utf8_span loc = {23, 5}; /* a = 5 */
    log_excerpt("some/file.dba", source, loc, "test");

    EXPECT_THAT(
        log_output,
        LogEq(" 2 | if a = 5 then a = 7\n"
              "   |    ^~~~< test\n"));
}

TEST_F(NAME, excerpt_wrap_to_next_line)
{
    const char* source
        = "for a = 1 to 10\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct utf8_span loc = {34, 15}; /* a = 7 ... print */
    log_excerpt("some/file.dba", source, loc, "");

    EXPECT_THAT(
        log_output,
        LogEq(" 2 | if a = 5 then a = 7\n"
              "   |               ^~~~~\n"
              " 3 | print str$(a)\n"
              "   | ~~~~< test\n"));
}

TEST_F(NAME, excerpt_multiple_lines)
{
    const char* source
        = "a = get ground height(\n"
          "       obj,\n"
          "    xpos,\n"
          "          zpos\n"
          ")\n";

    struct utf8_span loc = {30, 29}; /* 3 arguments */
    log_excerpt("some/file.dba", source, loc, "test");

    EXPECT_THAT(
        log_output,
        LogEq(" 2 |    obj,\n"
              "   |    ^~~~\n"
              " 3 | xpos,\n"
              "   | ~~~~~\n"
              " 4 |       zpos\n"
              "   | ~~~~~~~~~< test\n"));
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
    struct utf8_span op = {40, 1};
    struct utf8_span rhs = {42, 3};
    log_binop_excerpt(
        "some/file.dba", source, lhs, op, rhs, "variable", "constant");

    EXPECT_THAT(
        log_output,
        LogEq(" 2 | variable = variable and 333\n"
              "   |            >~~~~~~~ ^^^ ~~< constant\n"
              "   |            variable\n"));
}

TEST_F(NAME, excerpt_binop_wrap_to_next_line1)
{
    const char* source
        = "for a = 1 to 10\n"
          "    variable = variable\n"
          "                + 333\n"
          "    if a = 5 then a = 7\n"
          "    print str$(a)\n"
          "next a\n";

    struct utf8_span lhs = {31, 8};
    struct utf8_span op = {40, 1};
    struct utf8_span rhs = {42, 3};
    log_binop_excerpt(
        "some/file.dba", source, lhs, op, rhs, "variable", "constant");

    EXPECT_THAT(
        log_output,
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
    struct utf8_span op = {40, 1};
    struct utf8_span rhs = {42, 3};
    log_binop_excerpt(
        "some/file.dba", source, lhs, op, rhs, "variable", "constant");

    EXPECT_THAT(
        log_output,
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
    struct utf8_span op = {40, 1};
    struct utf8_span rhs = {42, 3};
    log_binop_excerpt(
        "some/file.dba", source, lhs, op, rhs, "variable", "constant");

    EXPECT_THAT(
        log_output,
        LogEq(" 2 | variable = variable\n"
              "   |            >~~~~~~~ variable\n"
              " 3 |               and\n"
              "   |               ^^^\n"
              " 4 |            333\n"
              "   |            ~~< constant\n"));
}

