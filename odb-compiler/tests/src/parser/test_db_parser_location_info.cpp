#include "odb-compiler/ast/Assignment.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/Command.hpp"
#include "odb-compiler/ast/Conditional.hpp"
#include "odb-compiler/ast/FuncCall.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/commands/Command.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"

#define NAME db_parser_location_info

using namespace testing;
using namespace odb;

class NAME : public ParserTestHarness
{
public:
};

TEST_F(NAME, oneline_function_call)
{
    ast = driver->parseString("test",
        "result = foo()");
    ASSERT_THAT(ast, NotNull());

    const auto& stmnts = ast->statements();
    ASSERT_THAT(stmnts.size(), Eq(1));
    ast::VarAssignment* ass = dynamic_cast<ast::VarAssignment*>(stmnts[0].get());
    ASSERT_THAT(ass, NotNull());
    ast::VarRef* result = dynamic_cast<ast::VarRef*>(ass->variable());
    ASSERT_THAT(result, NotNull());
    ast::AnnotatedSymbol* resultSym = dynamic_cast<ast::AnnotatedSymbol*>(result->symbol());
    ASSERT_THAT(resultSym, NotNull());
    ast::FuncCallExpr* foo = dynamic_cast<ast::FuncCallExpr*>(ass->expression());
    ASSERT_THAT(foo, NotNull());
    ast::AnnotatedSymbol* fooSym = dynamic_cast<ast::AnnotatedSymbol*>(foo->symbol());
    ASSERT_THAT(fooSym, NotNull());

    EXPECT_THAT(ast->location()->firstLine(), Eq(1));
    EXPECT_THAT(ast->location()->lastLine(), Eq(1));
    EXPECT_THAT(ast->location()->firstColumn(), Eq(1));
    EXPECT_THAT(ast->location()->lastColumn(), Eq(15));

    EXPECT_THAT(ass->location()->firstLine(), Eq(1));
    EXPECT_THAT(ass->location()->lastLine(), Eq(1));
    EXPECT_THAT(ass->location()->firstColumn(), Eq(1));
    EXPECT_THAT(ass->location()->lastColumn(), Eq(15));

    EXPECT_THAT(result->location()->firstLine(), Eq(1));
    EXPECT_THAT(result->location()->lastLine(), Eq(1));
    EXPECT_THAT(result->location()->firstColumn(), Eq(1));
    EXPECT_THAT(result->location()->lastColumn(), Eq(7));

    EXPECT_THAT(resultSym->location()->firstLine(), Eq(1));
    EXPECT_THAT(resultSym->location()->lastLine(), Eq(1));
    EXPECT_THAT(resultSym->location()->firstColumn(), Eq(1));
    EXPECT_THAT(resultSym->location()->lastColumn(), Eq(7));

    EXPECT_THAT(foo->location()->firstLine(), Eq(1));
    EXPECT_THAT(foo->location()->lastLine(), Eq(1));
    EXPECT_THAT(foo->location()->firstColumn(), Eq(10));
    EXPECT_THAT(foo->location()->lastColumn(), Eq(15));

    EXPECT_THAT(fooSym->location()->firstLine(), Eq(1));
    EXPECT_THAT(fooSym->location()->lastLine(), Eq(1));
    EXPECT_THAT(fooSym->location()->firstColumn(), Eq(10));
    EXPECT_THAT(fooSym->location()->lastColumn(), Eq(13));
}

TEST_F(NAME, print_command)
{
    ast = driver->parseString("test",
        "if x\n"
        "    foo()\n"
        "    bar()\n"
        "endif\n");
    ASSERT_THAT(ast, NotNull());

    const auto& stmnts = ast->statements();
    ASSERT_THAT(stmnts.size(), Eq(1));
    ast::Conditional* cond = dynamic_cast<ast::Conditional*>(stmnts[0].get());
    ASSERT_THAT(cond, NotNull());
    ast::VarRef* x = dynamic_cast<ast::VarRef*>(cond->condition());
    ASSERT_THAT(x, NotNull());
    ast::AnnotatedSymbol* xSym = dynamic_cast<ast::AnnotatedSymbol*>(x->symbol());
    ASSERT_THAT(xSym, NotNull());
    ast::Block* trueBranch = dynamic_cast<ast::Block*>(cond->trueBranch().get());
    ASSERT_THAT(trueBranch, NotNull());
    const auto& trueStmnts = trueBranch->statements();
    ASSERT_THAT(trueStmnts.size(), Eq(2));
    ast::FuncCallStmnt* foo = dynamic_cast<ast::FuncCallStmnt*>(trueStmnts[0].get());
    ASSERT_THAT(foo, NotNull());
    ast::AnnotatedSymbol* fooSym = dynamic_cast<ast::AnnotatedSymbol*>(foo->symbol());
    ASSERT_THAT(fooSym, NotNull());
    ast::FuncCallStmnt* bar = dynamic_cast<ast::FuncCallStmnt*>(trueStmnts[1].get());
    ASSERT_THAT(bar, NotNull());
    ast::AnnotatedSymbol* barSym = dynamic_cast<ast::AnnotatedSymbol*>(bar->symbol());
    ASSERT_THAT(barSym, NotNull());

    EXPECT_THAT(ast->location()->firstLine(), Eq(1));
    EXPECT_THAT(ast->location()->lastLine(), Eq(4));
    EXPECT_THAT(ast->location()->firstColumn(), Eq(1));
    EXPECT_THAT(ast->location()->lastColumn(), Eq(6));

    EXPECT_THAT(cond->location()->firstLine(), Eq(1));
    EXPECT_THAT(cond->location()->lastLine(), Eq(4));
    EXPECT_THAT(cond->location()->firstColumn(), Eq(1));
    EXPECT_THAT(cond->location()->lastColumn(), Eq(6));

    EXPECT_THAT(x->location()->firstLine(), Eq(1));
    EXPECT_THAT(x->location()->lastLine(), Eq(1));
    EXPECT_THAT(x->location()->firstColumn(), Eq(4));
    EXPECT_THAT(x->location()->lastColumn(), Eq(5));

    EXPECT_THAT(xSym->location()->firstLine(), Eq(1));
    EXPECT_THAT(xSym->location()->lastLine(), Eq(1));
    EXPECT_THAT(xSym->location()->firstColumn(), Eq(4));
    EXPECT_THAT(xSym->location()->lastColumn(), Eq(5));

    EXPECT_THAT(trueBranch->location()->firstLine(), Eq(2));
    EXPECT_THAT(trueBranch->location()->lastLine(), Eq(3));
    EXPECT_THAT(trueBranch->location()->firstColumn(), Eq(5));
    EXPECT_THAT(trueBranch->location()->lastColumn(), Eq(10));

    EXPECT_THAT(foo->location()->firstLine(), Eq(2));
    EXPECT_THAT(foo->location()->lastLine(), Eq(2));
    EXPECT_THAT(foo->location()->firstColumn(), Eq(5));
    EXPECT_THAT(foo->location()->lastColumn(), Eq(10));

    EXPECT_THAT(fooSym->location()->firstLine(), Eq(2));
    EXPECT_THAT(fooSym->location()->lastLine(), Eq(2));
    EXPECT_THAT(fooSym->location()->firstColumn(), Eq(5));
    EXPECT_THAT(fooSym->location()->lastColumn(), Eq(8));

    EXPECT_THAT(bar->location()->firstLine(), Eq(3));
    EXPECT_THAT(bar->location()->lastLine(), Eq(3));
    EXPECT_THAT(bar->location()->firstColumn(), Eq(5));
    EXPECT_THAT(bar->location()->lastColumn(), Eq(10));

    EXPECT_THAT(barSym->location()->firstLine(), Eq(3));
    EXPECT_THAT(barSym->location()->lastLine(), Eq(3));
    EXPECT_THAT(barSym->location()->firstColumn(), Eq(5));
    EXPECT_THAT(barSym->location()->lastColumn(), Eq(8));
}
