#include "gmock/gmock.h"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Assignment.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/commands/Command.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"

#define NAME db_parser_assignment

using namespace testing;
using namespace odb;

class NAME : public ParserTestHarness
{
public:
};

TEST_F(NAME, variable_with_assignment_has_default_type_integer)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test", "var = 5.4", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, float_variable_with_assignment_has_type_float)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test", "var# = 5.4", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::FLOAT, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, string_variable_with_assignment_has_type_string)
{
    using Annotation = ast::Symbol::Annotation;

    ast = driver->parse("test", "var$ = \"string\"", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::STRING, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitStringLiteral(StringLiteralEq("string"))).After(exp);

    ast->accept(&v);
}

/* NOTE: All UDT assignment cases are covered in test_db_parser_udt_fields.cpp */
