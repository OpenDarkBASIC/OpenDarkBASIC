#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/astpost/ResolveLabels.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/matchers/BlockStmntCountEq.hpp"
#include "odb-compiler/tests/matchers/GotoEqMatcher.hpp"
#include "odb-compiler/tests/matchers/IdentifierEq.hpp"
#include "odb-compiler/tests/matchers/SubCallEqMatcher.hpp"

#define NAME astpost_resolve_labels

using namespace testing;
using namespace odb;

class NAME : public ParserTestHarness
{
public:
};

TEST_F(NAME, resolve_gotos)
{
    cmdIndex.addCommand(new cmd::Command(nullptr, "command", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test",
                        "foo:\ncommand\nbar:\ncommand\ngoto foo\ngoto bar",
                        matcher);
    ASSERT_THAT(ast, NotNull());

    {
        StrictMock<ASTMockVisitor> v;
        Expectation exp;
        exp = EXPECT_CALL(v, visitProgram(_));
        exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(6))).After(exp);
        exp = EXPECT_CALL(v, visitLabel(_)).After(exp);
        exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("foo"))).After(exp);
        exp = EXPECT_CALL(v, visitCommandStmnt(_)).After(exp);
        exp = EXPECT_CALL(v, visitLabel(_)).After(exp);
        exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("bar"))).After(exp);
        exp = EXPECT_CALL(v, visitCommandStmnt(_)).After(exp);
        exp = EXPECT_CALL(v, visitUnresolvedGoto(_)).After(exp);
        exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("foo"))).After(exp);
        exp = EXPECT_CALL(v, visitUnresolvedGoto(_)).After(exp);
        exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("bar"))).After(exp);
        visitAST(ast, v);
    }

    astpost::ProcessGroup post;
    post.addProcess(std::make_unique<astpost::ResolveLabels>());
    ASSERT_THAT(post.execute(ast), IsTrue());

    {
        StrictMock<ASTMockVisitor> v;
        Expectation exp;
        exp = EXPECT_CALL(v, visitProgram(_));
        exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(6))).After(exp);
        exp = EXPECT_CALL(v, visitLabel(_)).After(exp);
        exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("foo"))).After(exp);
        exp = EXPECT_CALL(v, visitCommandStmnt(_)).After(exp);
        exp = EXPECT_CALL(v, visitLabel(_)).After(exp);
        exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("bar"))).After(exp);
        exp = EXPECT_CALL(v, visitCommandStmnt(_)).After(exp);
        exp = EXPECT_CALL(v, visitGoto(GotoEq("foo"))).After(exp);
        exp = EXPECT_CALL(v, visitGoto(GotoEq("bar"))).After(exp);
        visitAST(ast, v);
    }
}

TEST_F(NAME, resolve_gosubs)
{
    cmdIndex.addCommand(new cmd::Command(nullptr, "command", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test",
                        "gosub subr\nexit\nsubr:\ncommand\nreturn",
                        matcher);
    ASSERT_THAT(ast, NotNull());

    {
        StrictMock<ASTMockVisitor> v;
        Expectation exp;
        exp = EXPECT_CALL(v, visitProgram(_));
        exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(5))).After(exp);
        exp = EXPECT_CALL(v, visitUnresolvedSubCall(_)).After(exp);
        exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("subr"))).After(exp);
        exp = EXPECT_CALL(v, visitExit(_)).After(exp);
        exp = EXPECT_CALL(v, visitLabel(_)).After(exp);
        exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("subr"))).After(exp);
        exp = EXPECT_CALL(v, visitCommandStmnt(_)).After(exp);
        exp = EXPECT_CALL(v, visitSubReturn(_)).After(exp);
        visitAST(ast, v);
    }

    astpost::ProcessGroup post;
    post.addProcess(std::make_unique<astpost::ResolveLabels>());
    ASSERT_THAT(post.execute(ast), IsTrue());

    {
        StrictMock<ASTMockVisitor> v;
        Expectation exp;
        exp = EXPECT_CALL(v, visitProgram(_));
        exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(5))).After(exp);
        exp = EXPECT_CALL(v, visitSubCall(_)).After(exp);
        exp = EXPECT_CALL(v, visitExit(_)).After(exp);
        exp = EXPECT_CALL(v, visitLabel(_)).After(exp);
        exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("subr"))).After(exp);
        exp = EXPECT_CALL(v, visitCommandStmnt(_)).After(exp);
        exp = EXPECT_CALL(v, visitSubReturn(_)).After(exp);
        visitAST(ast, v);
    }
}
