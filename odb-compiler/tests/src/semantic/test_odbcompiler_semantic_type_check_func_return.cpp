#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"
#include "odb-util/tests/Utf8Helper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_integrity.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_func_return

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, missing_return_value)
{
    const char* source
        = "FUNCTION test() AS FLOAT\n"
          "ENDFUNCTION\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(-1));
    EXPECT_THAT(
        log(),
        LogEq("test:2:1\n"
              "error: Missing return value.\n"
              " 2 | ENDFUNCTION\n"
              "   | ^~~~~~~~~~<\n"
              "test:1:17\n"
              "note: Function return type was declared here:\n"
              " 1 | FUNCTION test() AS FLOAT\n"
              "   |                 ^~~~~~~<\n"));
}

TEST_F(NAME, return_value_but_func_is_void)
{
    const char* source
        = "FUNCTION test() AS VOID\n"
          "ENDFUNCTION 1\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(-1));
    EXPECT_THAT(
        log(),
        LogEq("test:2:13\n"
              "error: Cannot convert BYTE to VOID in function return. Types "
              "are incompatible.\n"
              " 2 | ENDFUNCTION 1\n"
              "   |             ^ BYTE\n"
              "test:1:17\n"
              "note: Function return type was declared here:\n"
              " 1 | FUNCTION test() AS VOID\n"
              "   |                 ^~~~~~<\n"));
}
