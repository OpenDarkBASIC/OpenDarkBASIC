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

    EXPECT_THAT(ast->location()->getFirstLine(), Eq(1));
    EXPECT_THAT(ast->location()->getLastLine(), Eq(1));
    EXPECT_THAT(ast->location()->getFirstColumn(), Eq(1));
    EXPECT_THAT(ast->location()->getLastColumn(), Eq(14));

    EXPECT_THAT(ass->location()->getFirstLine(), Eq(1));
    EXPECT_THAT(ass->location()->getLastLine(), Eq(1));
    EXPECT_THAT(ass->location()->getFirstColumn(), Eq(1));
    EXPECT_THAT(ass->location()->getLastColumn(), Eq(14));

    EXPECT_THAT(result->location()->getFirstLine(), Eq(1));
    EXPECT_THAT(result->location()->getLastLine(), Eq(1));
    EXPECT_THAT(result->location()->getFirstColumn(), Eq(1));
    EXPECT_THAT(result->location()->getLastColumn(), Eq(6));

    EXPECT_THAT(resultSym->location()->getFirstLine(), Eq(1));
    EXPECT_THAT(resultSym->location()->getLastLine(), Eq(1));
    EXPECT_THAT(resultSym->location()->getFirstColumn(), Eq(1));
    EXPECT_THAT(resultSym->location()->getLastColumn(), Eq(6));

    EXPECT_THAT(foo->location()->getFirstLine(), Eq(1));
    EXPECT_THAT(foo->location()->getLastLine(), Eq(1));
    EXPECT_THAT(foo->location()->getFirstColumn(), Eq(10));
    EXPECT_THAT(foo->location()->getLastColumn(), Eq(14));

    EXPECT_THAT(fooSym->location()->getFirstLine(), Eq(1));
    EXPECT_THAT(fooSym->location()->getLastLine(), Eq(1));
    EXPECT_THAT(fooSym->location()->getFirstColumn(), Eq(10));
    EXPECT_THAT(fooSym->location()->getLastColumn(), Eq(12));
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

    EXPECT_THAT(ast->location()->getFirstLine(), Eq(1));
    EXPECT_THAT(ast->location()->getLastLine(), Eq(4));
    EXPECT_THAT(ast->location()->getFirstColumn(), Eq(1));
    EXPECT_THAT(ast->location()->getLastColumn(), Eq(5));

    EXPECT_THAT(cond->location()->getFirstLine(), Eq(1));
    EXPECT_THAT(cond->location()->getLastLine(), Eq(4));
    EXPECT_THAT(cond->location()->getFirstColumn(), Eq(1));
    EXPECT_THAT(cond->location()->getLastColumn(), Eq(5));

    EXPECT_THAT(x->location()->getFirstLine(), Eq(1));
    EXPECT_THAT(x->location()->getLastLine(), Eq(1));
    EXPECT_THAT(x->location()->getFirstColumn(), Eq(4));
    EXPECT_THAT(x->location()->getLastColumn(), Eq(4));

    EXPECT_THAT(xSym->location()->getFirstLine(), Eq(1));
    EXPECT_THAT(xSym->location()->getLastLine(), Eq(1));
    EXPECT_THAT(xSym->location()->getFirstColumn(), Eq(4));
    EXPECT_THAT(xSym->location()->getLastColumn(), Eq(4));

    EXPECT_THAT(trueBranch->location()->getFirstLine(), Eq(2));
    EXPECT_THAT(trueBranch->location()->getLastLine(), Eq(3));
    EXPECT_THAT(trueBranch->location()->getFirstColumn(), Eq(5));
    EXPECT_THAT(trueBranch->location()->getLastColumn(), Eq(9));

    EXPECT_THAT(foo->location()->getFirstLine(), Eq(2));
    EXPECT_THAT(foo->location()->getLastLine(), Eq(2));
    EXPECT_THAT(foo->location()->getFirstColumn(), Eq(5));
    EXPECT_THAT(foo->location()->getLastColumn(), Eq(9));

    EXPECT_THAT(fooSym->location()->getFirstLine(), Eq(2));
    EXPECT_THAT(fooSym->location()->getLastLine(), Eq(2));
    EXPECT_THAT(fooSym->location()->getFirstColumn(), Eq(5));
    EXPECT_THAT(fooSym->location()->getLastColumn(), Eq(7));

    EXPECT_THAT(bar->location()->getFirstLine(), Eq(3));
    EXPECT_THAT(bar->location()->getLastLine(), Eq(3));
    EXPECT_THAT(bar->location()->getFirstColumn(), Eq(5));
    EXPECT_THAT(bar->location()->getLastColumn(), Eq(9));

    EXPECT_THAT(barSym->location()->getFirstLine(), Eq(3));
    EXPECT_THAT(barSym->location()->getLastLine(), Eq(3));
    EXPECT_THAT(barSym->location()->getFirstColumn(), Eq(5));
    EXPECT_THAT(barSym->location()->getLastColumn(), Eq(7));
}
