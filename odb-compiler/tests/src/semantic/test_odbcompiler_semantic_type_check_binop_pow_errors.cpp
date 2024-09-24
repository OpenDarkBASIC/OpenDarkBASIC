#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_binop_pow_errors

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, exponent_invalid_type)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_DOUBLE});
    ASSERT_THAT(parse("print 2.0 ^ \"oops\""), Eq(0));
    EXPECT_THAT(
        semantic_check_run(
            &semantic_type_check, &ast, plugins, &cmds, symbols, "test", src),
        Eq(-1));
    EXPECT_THAT(
        log(),
        LogEq("test:1:13: error: Incompatible exponent type STRING can't be "
              "converted to INTEGER.\n"
              " 1 | print 2.0 ^ \"oops\"\n"
              "   |       >~~ ^ ~~~~~< STRING\n"
              "   = note: The exponent can be an INTEGER, FLOAT or DOUBLE.\n"));
}
