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
        = "TYPE Test\n"
          "    x AS INTEGER\n"
          "    y AS INTEGER\n"
          "ENDTYPE\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
}

TEST_F(NAME, nested_decl)
{
    const char* source
        = "TYPE Test\n"
          "    x AS INTEGER\n"
          "    TYPE Bar\n"
          "        a AS FLOAT\n"
          "        b AS FLOAT\n"
          "    ENDTYPE\n"
          "    y AS INTEGER\n"
          "ENDTYPE\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
}

TEST_F(NAME, as_udt)
{
    const char* source
        = "TYPE Test\n"
          "    x AS INTEGER\n"
          "    y AS INTEGER\n"
          "ENDTYPE\n"
          "var AS Test\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
}

TEST_F(NAME, read_udt_field)
{
    const char* source=
          "a = var.x.z.y\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
}
