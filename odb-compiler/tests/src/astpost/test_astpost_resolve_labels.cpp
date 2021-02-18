#include "gmock/gmock.h"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/astpost/ResolveLabels.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"

#define NAME astpost_resolve_labels

using namespace testing;
using namespace odb;

class NAME : public ParserTestHarness
{
public:
};

TEST_F(NAME, single_goto_label)
{
    ast = driver->parse("test",
        "goto l1\n"
        "l1:\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(2)));
    exp = EXPECT_CALL(v, visitGotoSymbol(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("l1"))).After(exp);
    exp = EXPECT_CALL(v, visitLabel(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("l1"))).After(exp);
    ast->accept(&v);

    astpost::ResolveLabels rl;
    ASSERT_THAT(rl.execute(ast), IsTrue());

    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(2)));
    exp = EXPECT_CALL(v, visitGoto(GotoEq("l1"))).After(exp);
    exp = EXPECT_CALL(v, visitLabel(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("l1"))).After(exp);
    ast->accept(&v);
}

TEST_F(NAME, single_goto_label_doesnt_match)
{
    ast = driver->parse("test",
        "goto l1\n"
        "l2:\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(2)));
    exp = EXPECT_CALL(v, visitGotoSymbol(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("l1"))).After(exp);
    exp = EXPECT_CALL(v, visitLabel(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("l2"))).After(exp);
    ast->accept(&v);

    astpost::ResolveLabels rl;
    ASSERT_THAT(rl.execute(ast), IsFalse());
}

TEST_F(NAME, label_redefinition)
{
    ast = driver->parse("test",
        "l1:\n"
        "l1:\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(2)));
    exp = EXPECT_CALL(v, visitLabel(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("l1"))).After(exp);
    exp = EXPECT_CALL(v, visitLabel(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("l1"))).After(exp);
    ast->accept(&v);

    astpost::ResolveLabels rl;
    ASSERT_THAT(rl.execute(ast), IsFalse());
}
