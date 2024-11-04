#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_udt

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, decl)
{
    const char* source
        = "TYPE Foo\n"
          "    x AS INTEGER\n"
          "    y# AS FLOAT\n"
          "ENDTYPE\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
}

TEST_F(NAME, as_udt)
{
    const char* source
        = "TYPE Foo\n"
          "    x AS INTEGER\n"
          "    y# AS FLOAT\n"
          "ENDTYPE\n"
          "foo AS Foo\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
}

TEST_F(NAME, read_udt_field)
{
    const char* source
        = "TYPE Foo\n"
          "    x AS INTEGER\n"
          "    y# AS FLOAT\n"
          "ENDTYPE\n"
          "foo AS Foo\n"
          "a = foo.x\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
}

TEST_F(NAME, write_udt_field)
{
    const char* source
        = "TYPE Foo\n"
          "    x AS INTEGER\n"
          "    y# AS FLOAT\n"
          "ENDTYPE\n"
          "foo AS Foo\n"
          "foo.x = 5\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
}

TEST_F(NAME, nested_decl)
{
    const char* source
        = "TYPE Bar\n"
          "    a AS WORD\n"
          "    b AS DWORD\n"
          "ENDTYPE\n"
          "TYPE Foo\n"
          "    x AS INTEGER\n"
          "    bar AS Bar\n"
          "    y# AS FLOAT\n"
          "ENDTYPE\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
}

TEST_F(NAME, nested_as_udt)
{
    const char* source
        = "TYPE Bar\n"
          "    a AS WORD\n"
          "    b AS DWORD\n"
          "ENDTYPE\n"
          "TYPE Foo\n"
          "    x AS INTEGER\n"
          "    bar AS Bar\n"
          "    y# AS FLOAT\n"
          "ENDTYPE\n"
          "foo AS Foo\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
}

TEST_F(NAME, nested_read_udt_field)
{
    const char* source
        = "TYPE Bar\n"
          "    a AS WORD\n"
          "    b AS DWORD\n"
          "ENDTYPE\n"
          "TYPE Foo\n"
          "    x AS INTEGER\n"
          "    bar AS Bar\n"
          "    y# AS FLOAT\n"
          "ENDTYPE\n"
          "foo AS Foo\n"
          "a = foo.bar.b\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
}

TEST_F(NAME, nested_write_udt_field)
{
    const char* source
        = "TYPE Bar\n"
          "    a AS WORD\n"
          "    b AS DWORD\n"
          "ENDTYPE\n"
          "TYPE Foo\n"
          "    x AS INTEGER\n"
          "    bar AS Bar\n"
          "    y# AS FLOAT\n"
          "ENDTYPE\n"
          "foo AS Foo\n"
          "foo.bar.b = 5\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
}
