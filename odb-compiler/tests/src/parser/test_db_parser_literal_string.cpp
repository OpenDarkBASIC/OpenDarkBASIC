#include "odb-compiler/ast/Annotation.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/matchers/AnnotatedSymbolEq.hpp"
#include "odb-compiler/tests/matchers/BlockStmntCountEq.hpp"
#include "odb-compiler/tests/matchers/LiteralEq.hpp"

#define NAME db_parser_literal_string

using namespace testing;
using namespace odb;
using namespace ast;

class NAME : public ParserTestHarness
{
public:
};

TEST_F(NAME, simple_string_assignment)
{
    ast = driver->parse("test",
        "a = \"test\"",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, string_with_backslash)
{
    ast = driver->parse("test",
        "a = \"test\\\"",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, string_with_path_backslashes)
{
    ast = driver->parse("test",
        "a = \"path\\to\\file\"",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, string_append_filename_with_backslash)
{
    ast = driver->parse("test",
        "if foo(\"maps\\\" + LevelEditor.name$) then bar(\"maps\\\" + LevelEditor.name$)",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, empty_string)
{
    ast = driver->parse("test",
        "x = \"\"",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitStringLiteral(StringLiteralEq(""))).After(exp);

    visitAST(ast, v);
}
