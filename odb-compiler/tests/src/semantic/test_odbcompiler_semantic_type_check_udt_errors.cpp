#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_udt_errors

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, read_nonexistent_member)
{
    const char* source
        = "TYPE Foo\n"
          "    x AS INTEGER\n"
          "    y# AS FLOAT\n"
          "ENDTYPE\n"
          "foo AS Foo\n"
          "a = foo.z\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(-1));
    EXPECT_THAT(
        log(),
        LogEq("test:6:9\n"
              "error: Member `z' not found in User-Defined Type `Foo'.\n"
              " 6 | a = foo.z\n"
              "   |         ^\n"
              "test:5:1\n"
              "note: `foo' was declared here.\n"
              " 5 | foo AS Foo\n"
              "   | ^~<    ^~<\n"
              "test:1:6\n"
              "note: `Foo' is defined here.\n"
              " 1 | TYPE Foo\n"
              "   |      ^~<\n"));
}

TEST_F(NAME, read_nonexistent_udt)
{
    const char* source
        = "TYPE Foo\n"
          "    x AS INTEGER\n"
          "    y# AS FLOAT\n"
          "ENDTYPE\n"
          "a = bar.z\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(-1));
    EXPECT_THAT(
        log(),
        LogEq("test:6:9\n"
              "error: `bar' is not a User-Defined Type.\n"
              " 6 | a = bar.z\n"
              "   |     ^~<\n"));
}

TEST_F(NAME, read_variable_as_udt)
{
    const char* source
        = "TYPE Foo\n"
          "    x AS INTEGER\n"
          "    y# AS FLOAT\n"
          "ENDTYPE\n"
          "bar AS INTEGER\n"
          "a = bar.z\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(-1));
    EXPECT_THAT(
        log(),
        LogEq("test:6:9\n"
              "error: `bar' is not a User-Defined Type.\n"
              " 6 | a = bar.z\n"
              "   |     ^~<\n"
              "test:5:1\n"
              "note: `bar' was declared here.\n"
              " 5 | bar AS INTEGER\n"
              "   |        ^~~~~~<\n"));
}
