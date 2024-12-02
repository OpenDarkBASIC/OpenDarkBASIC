#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_integrity.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_select

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, case_implicit_conversion)
{
    const char* source
        = "SELECT 5.6\n"
          "    CASE 42\n"
          "    ENDCASE\n"
          "ENDSELECT\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;

    EXPECT_THAT(
        log(),
        LogEq("test:2:10\n"
              "warning: Implicit conversion from BYTE to DOUBLE in select "
              "statement.\n"
              " 2 | CASE 42\n"
              "   |      ^< BYTE\n"
              " 1 | SELECT 5.6\n"
              "   |        ^~< DOUBLE\n"
              "help: Insert an explicit cast to silence this warning:\n"
              " 2 | CASE 42 AS DOUBLE\n"
              "   |        ^~~~~~~~~<\n"));
}

TEST_F(NAME, select_void_func)
{
    const char* source
        = "SELECT test()\n"
          "ENDSELECT\n"
          "FUNCTION test() AS VOID\n"
          "ENDFUNCTION\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_select), Eq(-1));

    // TODO log output
}
