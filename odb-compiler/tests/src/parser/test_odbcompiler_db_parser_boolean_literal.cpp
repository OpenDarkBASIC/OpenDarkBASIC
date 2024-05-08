#include "odb-compiler/tests/DBParserTestHarness.hpp"

#define NAME odbcompiler_db_parser_literal_bool

using namespace testing;

class NAME : public DBParserTestHarness
{
public:
};

TEST_F(NAME, bool_true)
{
    ASSERT_THAT(parse("#constant a true\n"), Eq(0));

    /*
    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBooleanLiteral(BooleanLiteralEq(true))).After(exp);

    visitAST(ast, v);*/
}

TEST_F(NAME, bool_false)
{
    ASSERT_THAT(parse("#constant a false\n"), Eq(0));

/*
    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitConstDeclExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitBooleanLiteral(BooleanLiteralEq(false))).After(exp);

    visitAST(ast, v);*/
}

TEST_F(NAME, two_statements)
{
    ASSERT_THAT(parse("#constant a false\n#constant b true\n"), Eq(0));
}
