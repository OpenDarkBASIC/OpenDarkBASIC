#include "gmock/gmock.h"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"

#define NAME db_parser_string_literal

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odb;
using namespace ast;

TEST_F(NAME, simple_string_assignment)
{
    ast = driver->parseString("test",
        "a = \"test\"");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, string_with_backslash)
{
    ast = driver->parseString("test",
        "a = \"test\\\"");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, string_with_path_backslashes)
{
    ast = driver->parseString("test",
        "a = \"path\\to\\file\"");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, string_append_filename_with_backslash)
{
    ast = driver->parseString("test",
        "if foo(\"maps\\\" + LevelEditor.name$) then bar(\"maps\\\" + LevelEditor.name$)");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, empty_string)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parseString("test",
        "x = \"\"");
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
