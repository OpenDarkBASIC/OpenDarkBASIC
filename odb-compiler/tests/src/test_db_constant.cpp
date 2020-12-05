#include <gmock/gmock.h>
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME db_constant

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odb;

TEST_F(NAME, bool_constant)
{
    ast = driver->parseString("test",
        "#constant mybool1 true\n"
        "#constant mybool2 false\n");
    ASSERT_THAT(ast, NotNull());
/*
    ast::Node* block = ast;
    ASSERT_THAT(block->info.type, Eq(ast::NT_BLOCK));
    ASSERT_THAT(block->block.stmnt->info.type, Eq(ast::NT_SYM_CONST_DECL));
    ASSERT_THAT(block->block.stmnt->sym.const_decl.name, StrEq("mybool1"));
    ASSERT_THAT(block->block.stmnt->sym.const_decl.literal->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(block->block.stmnt->sym.const_decl.literal->literal.type, Eq(ast::LT_BOOLEAN));
    ASSERT_THAT(block->block.stmnt->sym.const_decl.literal->literal.value.b, IsTrue());

    block = block->block.next;
    ASSERT_THAT(block->info.type, Eq(ast::NT_BLOCK));
    ASSERT_THAT(block->block.stmnt->info.type, Eq(ast::NT_SYM_CONST_DECL));
    ASSERT_THAT(block->block.stmnt->sym.const_decl.name, StrEq("mybool2"));
    ASSERT_THAT(block->block.stmnt->sym.const_decl.literal->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(block->block.stmnt->sym.const_decl.literal->literal.type, Eq(ast::LT_BOOLEAN));
    ASSERT_THAT(block->block.stmnt->sym.const_decl.literal->literal.value.b, IsFalse());*/
}

TEST_F(NAME, integer_constant)
{
    ast = driver->parseString("test",
        "#constant myint 20\n");
    ASSERT_THAT(ast, NotNull());
/*
    ASSERT_THAT(ast->info.type, Eq(ast::NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(ast::NT_SYM_CONST_DECL));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.name, StrEq("myint"));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->literal.type, Eq(ast::LT_INTEGER));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->literal.value.i, Eq(20));*/
}

TEST_F(NAME, float_constant)
{
    ast = driver->parseString("test",
        "#constant myfloat 5.2\n");
    ASSERT_THAT(ast, NotNull());
/*
    ASSERT_THAT(ast->info.type, Eq(ast::NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(ast::NT_SYM_CONST_DECL));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.name, StrEq("myfloat"));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->literal.type, Eq(ast::LT_FLOAT));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->literal.value.f, DoubleEq(5.2));*/
}

TEST_F(NAME, string_constant)
{
    ast = driver->parseString("test",
        "#constant mystring \"hello world!\"\n");
    ASSERT_THAT(ast, NotNull());
/*
    ASSERT_THAT(ast->info.type, Eq(ast::NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(ast::NT_SYM_CONST_DECL));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.name, StrEq("mystring"));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->literal.type, Eq(ast::LT_STRING));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->literal.value.s, StrEq("hello world!"));*/
}

TEST_F(NAME, more_than_one_value_fails)
{
    EXPECT_THAT(driver->parseString("test",
        "#constant fail1 true false\n"), IsNull());
    EXPECT_THAT(driver->parseString("test",
        "#constant fail2 23 3.4\n"), IsNull());
}

TEST_F(NAME, float_1_notation)
{
    ast = driver->parseString("test",
        "#constant myfloat 53.\n");
    ASSERT_THAT(ast, NotNull());
/*
    ASSERT_THAT(ast->info.type, Eq(ast::NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(ast::NT_SYM_CONST_DECL));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.name, StrEq("myfloat"));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->literal.type, Eq(ast::LT_FLOAT));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->literal.value.f, DoubleEq(53));*/
}

TEST_F(NAME, float_2_notation)
{
    ast = driver->parseString("test",
        "#constant myfloat .53\n");
    ASSERT_THAT(ast, NotNull());
/*
    ASSERT_THAT(ast->info.type, Eq(ast::NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(ast::NT_SYM_CONST_DECL));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.name, StrEq("myfloat"));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->literal.type, Eq(ast::LT_FLOAT));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->literal.value.f, DoubleEq(.53));*/
}

TEST_F(NAME, float_3_notation)
{
    ast = driver->parseString("test",
        "#constant myfloat 53.f\n");
    ASSERT_THAT(ast, NotNull());
/*
    ASSERT_THAT(ast->info.type, Eq(ast::NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(ast::NT_SYM_CONST_DECL));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.name, StrEq("myfloat"));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->literal.type, Eq(ast::LT_FLOAT));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->literal.value.f, DoubleEq(53));*/
}

TEST_F(NAME, float_4_notation)
{
    ast = driver->parseString("test",
        "#constant myfloat .53f\n");
    ASSERT_THAT(ast, NotNull());
/*
    ASSERT_THAT(ast->info.type, Eq(ast::NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(ast::NT_SYM_CONST_DECL));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.name, StrEq("myfloat"));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->literal.type, Eq(ast::LT_FLOAT));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->literal.value.f, DoubleEq(.53));*/
}

TEST_F(NAME, float_exponential_notation_1)
{
    ast = driver->parseString("test",
        "#constant myfloat 12e2\n");
    ASSERT_THAT(ast, NotNull());
/*
    ASSERT_THAT(ast->info.type, Eq(ast::NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(ast::NT_SYM_CONST_DECL));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.name, StrEq("myfloat"));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->literal.type, Eq(ast::LT_FLOAT));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->literal.value.f, DoubleEq(1200));*/
}

TEST_F(NAME, float_exponential_notation_2)
{
    ast = driver->parseString("test",
        "#constant myfloat 12.4e2\n");
    ASSERT_THAT(ast, NotNull());
/*
    ASSERT_THAT(ast->info.type, Eq(ast::NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(ast::NT_SYM_CONST_DECL));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.name, StrEq("myfloat"));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->literal.type, Eq(ast::LT_FLOAT));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->literal.value.f, DoubleEq(1240));*/
}

TEST_F(NAME, float_exponential_notation_3)
{
    ast = driver->parseString("test",
        "#constant myfloat .4e2\n");
    ASSERT_THAT(ast, NotNull());
/*
    ASSERT_THAT(ast->info.type, Eq(ast::NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(ast::NT_SYM_CONST_DECL));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.name, StrEq("myfloat"));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->literal.type, Eq(ast::LT_FLOAT));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->literal.value.f, DoubleEq(40));*/
}

TEST_F(NAME, float_exponential_notation_4)
{
    ast = driver->parseString("test",
        "#constant myfloat 43.e2\n");
    ASSERT_THAT(ast, NotNull());
/*
    ASSERT_THAT(ast->info.type, Eq(ast::NT_BLOCK));
    ASSERT_THAT(ast->block.stmnt->info.type, Eq(ast::NT_SYM_CONST_DECL));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.name, StrEq("myfloat"));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->info.type, Eq(ast::NT_LITERAL));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->literal.type, Eq(ast::LT_FLOAT));
    ASSERT_THAT(ast->block.stmnt->sym.const_decl.literal->literal.value.f, DoubleEq(4300));*/
}
