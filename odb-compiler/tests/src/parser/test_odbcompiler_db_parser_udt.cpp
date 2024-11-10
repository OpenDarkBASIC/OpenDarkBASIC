#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include <gmock/gmock.h>

extern "C" {
#include "odb-compiler/ast/ast.h"
}

#define NAME odbcompiler_db_parser_udt

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, nested_decl_invalid_syntax)
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
    ASSERT_THAT(parse(source), Eq(-1)) << log().text;
    EXPECT_THAT(log(), LogEq(
        "test:3:5\n"
        "error: Nested User-Defined Types are not supported.\n"
        " 3 | TYPE Bar\n"
        "   | ^~~~\n"));
}

