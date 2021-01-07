#include "odb-compiler/ast/Assignment.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/Command.hpp"
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

TEST_F(NAME, print_command)
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
