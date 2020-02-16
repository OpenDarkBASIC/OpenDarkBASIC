#include <gmock/gmock.h>
#include "odbc/Driver.hpp"
#include "odbc/ASTNode.hpp"
#include <fstream>

#define NAME ast_function_call

using namespace testing;

class NAME : public Test
{
public:
    void SetUp() override { driver = new odbc::Driver; }
    void TearDown() override { delete driver; }
    odbc::Driver* driver;
};

using namespace odbc;

TEST_F(NAME, function_call_no_args)
{
    ASSERT_THAT(driver->parseString("foo()\n"), IsTrue());

    ASSERT_THAT(driver->getAST()->info.type, Eq(ast::NT_BLOCK));
    ASSERT_THAT(driver->getAST()->block.statement->info.type, Eq(ast::NT_SYMBOL_REF));
    ASSERT_THAT(driver->getAST()->block.statement->symbol_ref.type, Eq(ast::ST_FUNC_CALL));
    ASSERT_THAT(driver->getAST()->block.statement->symbol_ref.name, StrEq("foo"));
    ASSERT_THAT(driver->getAST()->block.statement->symbol_ref.arglist, IsNull());
}
