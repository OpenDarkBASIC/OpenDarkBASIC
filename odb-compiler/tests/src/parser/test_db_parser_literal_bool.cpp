#include "odb-compiler/ast/Annotation.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/matchers/BlockStmntCountEq.hpp"
#include "odb-compiler/tests/matchers/IdentifierEq.hpp"
#include "odb-compiler/tests/matchers/LiteralEq.hpp"
#include "gmock/gmock.h"

#define NAME db_parser_literal_bool

using namespace testing;
using namespace odb;
using namespace ast;

class NAME : public ParserTestHarness
{
public:
};

TEST_F(NAME, bool_true)
{
    ast = driver->parse("test",
        "#constant a true\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBooleanLiteral(BooleanLiteralEq(true))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, bool_false)
{
    ast = driver->parse("test",
        "#constant a false\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBooleanLiteral(BooleanLiteralEq(false))).After(exp);

    visitAST(ast, v);
}
