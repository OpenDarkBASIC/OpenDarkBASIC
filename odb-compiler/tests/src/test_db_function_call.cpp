#include <gmock/gmock.h>
#include "odbc/parsers/db/Driver.hpp"
#include "odbc/ast/Node.hpp"
#include "odbc/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME db_function_call

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odbc;

TEST_F(NAME, function_call_no_args)
{
    ASSERT_THAT(driver->parseString("foo()\n"), IsTrue());

    ASSERT_THAT(ast->info.type, Eq(ast::NT_BLOCK));
    ASSERT_THAT(ast->block.statement->info.type, Eq(ast::NT_SYM_FUNC_CALL));
}

TEST_F(NAME, function_call_no_args_string_return_type)
{
    ASSERT_THAT(driver->parseString("foo$()\n"), IsTrue());

    ASSERT_THAT(ast->info.type, Eq(ast::NT_BLOCK));
    ASSERT_THAT(ast->block.statement->info.type, Eq(ast::NT_SYM_FUNC_CALL));
}

TEST_F(NAME, function_call_no_args_float_return_type)
{
    ASSERT_THAT(driver->parseString("foo#()\n"), IsTrue());

    ASSERT_THAT(ast->info.type, Eq(ast::NT_BLOCK));
    ASSERT_THAT(ast->block.statement->info.type, Eq(ast::NT_SYM_FUNC_CALL));
}
