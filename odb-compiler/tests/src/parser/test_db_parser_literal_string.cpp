#include "gmock/gmock.h"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"

#define NAME db_parser_literal_string

using namespace testing;
using namespace odb;

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
    using Annotation = ast::Symbol::Annotation;

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

    ast->accept(&v);
}
