#include <gmock/gmock.h>
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME db_function_call

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odb;

TEST_F(NAME, function_call_no_args)
{
    ASSERT_THAT(driver->parseString("foo()\n"), IsTrue());

    ASSERT_THAT(ast->info.type, Eq(ast::NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(ast::NT_SYM_FUNC_CALL));
}

TEST_F(NAME, function_call_no_args_string_return_type)
{
    ASSERT_THAT(driver->parseString("foo$()\n"), IsTrue());

    ASSERT_THAT(ast->info.type, Eq(ast::NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(ast::NT_SYM_FUNC_CALL));
}

TEST_F(NAME, function_call_no_args_float_return_type)
{
    ASSERT_THAT(driver->parseString("foo#()\n"), IsTrue());

    ASSERT_THAT(ast->info.type, Eq(ast::NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(ast::NT_SYM_FUNC_CALL));
}
