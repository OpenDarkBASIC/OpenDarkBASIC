#include <gmock/gmock.h>
#include "odbc/Driver.hpp"
#include "odbc/ASTNode.hpp"
#define NAME constant
#include <fstream>

using namespace testing;

class NAME : public Test
{
public:
    void SetUp() override { driver = new odbc::Driver; }
    void TearDown() override { delete driver; }
    odbc::Driver* driver;
};

using namespace odbc;

TEST_F(NAME, bool_constant)
{
    ASSERT_THAT(driver->parseString(
        "#constant mybool1 true\n"
        "#constant mybool2 false\n"), IsTrue());

    ast::node_t* block = driver->getAST();
    ASSERT_THAT(block->info.type, Eq(ast::BLOCK));
    ASSERT_THAT(block->block.statement->info.type, Eq(ast::SYMBOL));
    ASSERT_THAT(block->block.statement->symbol.value->info.type, Eq(ast::BOOLEAN_CONSTANT));
    ASSERT_THAT(block->block.statement->symbol.value->boolean_constant.value, IsTrue());

    block = block->block.next;
    ASSERT_THAT(block, NotNull());
    ASSERT_THAT(block->info.type, Eq(ast::BLOCK));
    ASSERT_THAT(block->block.statement->info.type, Eq(ast::SYMBOL));
    ASSERT_THAT(block->block.statement->symbol.value->info.type, Eq(ast::BOOLEAN_CONSTANT));
    ASSERT_THAT(block->block.statement->symbol.value->boolean_constant.value, IsFalse());
}

TEST_F(NAME, integer_constant)
{
    EXPECT_THAT(driver->parseString(
        "#constant myint 20\n"), Eq(true));
}

TEST_F(NAME, float_constant)
{
    EXPECT_THAT(driver->parseString(
        "#constant myfloat 5.2\n"), Eq(true));
}

TEST_F(NAME, string_constant)
{
    EXPECT_THAT(driver->parseString(
        "#constant mystring \"hello world!\"\n"), Eq(true));
}

TEST_F(NAME, more_than_one_value_fails)
{
    EXPECT_THAT(driver->parseString(
        "#constant fail1 true false\n"), Eq(false));
    EXPECT_THAT(driver->parseString(
        "#constant fail2 23 3.4\n"), Eq(false));
}

TEST_F(NAME, cant_define_same_symbol_twice)
{
    EXPECT_THAT(driver->parseString(
        "#constant mybool true\n"), Eq(true));
    EXPECT_THAT(driver->parseString(
        "#constant mybool true\n"), Eq(false));
}

TEST_F(NAME, float_1_notation)
{
    ASSERT_THAT(driver->parseString(
        "#constant myfloat 53.\n"), Eq(true));
    ASSERT_THAT(driver->getAST()->info.type, Eq(ast::BLOCK));
    ASSERT_THAT(driver->getAST()->block.statement->info.type, Eq(ast::SYMBOL));
    ASSERT_THAT(driver->getAST()->block.statement->symbol.value->info.type, Eq(ast::FLOAT_CONSTANT));
    ASSERT_THAT(driver->getAST()->block.statement->symbol.value->float_constant.value, DoubleEq(53.0));
}

TEST_F(NAME, float_2_notation)
{
    ASSERT_THAT(driver->parseString(
        "#constant myfloat .53\n"), Eq(true));
    ASSERT_THAT(driver->getAST()->info.type, Eq(ast::BLOCK));
    ASSERT_THAT(driver->getAST()->block.statement->info.type, Eq(ast::SYMBOL));
    ASSERT_THAT(driver->getAST()->block.statement->symbol.value->info.type, Eq(ast::FLOAT_CONSTANT));
    ASSERT_THAT(driver->getAST()->block.statement->symbol.value->float_constant.value, DoubleEq(0.53));
}

TEST_F(NAME, float_3_notation)
{
    ASSERT_THAT(driver->parseString(
        "#constant myfloat 53.f\n"), Eq(true));
    ASSERT_THAT(driver->getAST()->info.type, Eq(ast::BLOCK));
    ASSERT_THAT(driver->getAST()->block.statement->info.type, Eq(ast::SYMBOL));
    ASSERT_THAT(driver->getAST()->block.statement->symbol.value->info.type, Eq(ast::FLOAT_CONSTANT));
    ASSERT_THAT(driver->getAST()->block.statement->symbol.value->float_constant.value, DoubleEq(53.0));
}

TEST_F(NAME, float_4_notation)
{
    ASSERT_THAT(driver->parseString(
        "#constant myfloat .53f\n"), Eq(true));
    ASSERT_THAT(driver->getAST()->info.type, Eq(ast::BLOCK));
    ASSERT_THAT(driver->getAST()->block.statement->info.type, Eq(ast::SYMBOL));
    ASSERT_THAT(driver->getAST()->block.statement->symbol.value->info.type, Eq(ast::FLOAT_CONSTANT));
    ASSERT_THAT(driver->getAST()->block.statement->symbol.value->float_constant.value, DoubleEq(0.53));
}

TEST_F(NAME, float_exponential_notation_1)
{
    ASSERT_THAT(driver->parseString(
        "#constant myfloat 12e2\n"), Eq(true));
    ASSERT_THAT(driver->getAST()->info.type, Eq(ast::BLOCK));
    ASSERT_THAT(driver->getAST()->block.statement->info.type, Eq(ast::SYMBOL));
    ASSERT_THAT(driver->getAST()->block.statement->symbol.value->info.type, Eq(ast::FLOAT_CONSTANT));
    ASSERT_THAT(driver->getAST()->block.statement->symbol.value->float_constant.value, DoubleEq(1200));
}

TEST_F(NAME, float_exponential_notation_2)
{
    ASSERT_THAT(driver->parseString(
        "#constant myfloat 12.4e2\n"), Eq(true));
    ASSERT_THAT(driver->getAST()->info.type, Eq(ast::BLOCK));
    ASSERT_THAT(driver->getAST()->block.statement->info.type, Eq(ast::SYMBOL));
    ASSERT_THAT(driver->getAST()->block.statement->symbol.value->info.type, Eq(ast::FLOAT_CONSTANT));
    ASSERT_THAT(driver->getAST()->block.statement->symbol.value->float_constant.value, DoubleEq(1240));
}

TEST_F(NAME, float_exponential_notation_3)
{
    ASSERT_THAT(driver->parseString(
        "#constant myfloat .4e2\n"), Eq(true));
    ASSERT_THAT(driver->getAST()->info.type, Eq(ast::BLOCK));
    ASSERT_THAT(driver->getAST()->block.statement->info.type, Eq(ast::SYMBOL));
    ASSERT_THAT(driver->getAST()->block.statement->symbol.value->info.type, Eq(ast::FLOAT_CONSTANT));
    ASSERT_THAT(driver->getAST()->block.statement->symbol.value->float_constant.value, DoubleEq(40));
}

TEST_F(NAME, float_exponential_notation_4)
{
    ASSERT_THAT(driver->parseString(
        "#constant myfloat 43.e2\n"), Eq(true));
    ASSERT_THAT(driver->getAST()->info.type, Eq(ast::BLOCK));
    ASSERT_THAT(driver->getAST()->block.statement->info.type, Eq(ast::SYMBOL));
    ASSERT_THAT(driver->getAST()->block.statement->symbol.value->info.type, Eq(ast::FLOAT_CONSTANT));
    ASSERT_THAT(driver->getAST()->block.statement->symbol.value->float_constant.value, DoubleEq(4300));
}
