#include "odb-compiler/astpost/ValidateUDTFieldNames.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"

#define NAME db_parser_udt_fields

using namespace testing;
using namespace odb;

class NAME : public ParserTestHarness
{
public:
};

/*
 * All possible valid UDT field refs
 *
 *     a.b.c = value
 *     a.b.c# = value
 *     a.b.c$ = value
 *     a.b.c(x) = value
 *     a.b.c#(x) = value
 *     a.b.c$(x) = value
 *     a.b(x).c = value
 *     a(x).b.c = value
 *
 *     value = a.b.c
 *     value = a.b.c#
 *     value = a.b.c$
 *     value = a.b.c(x)
 *     value = a.b.c#(x)
 *     value = a.b.c$(x)
 *     value = a.b(x).c
 *     value = a(x).b.c
 *     value = func_expr().b.c
 *     value = cmd expr().b.c
 *
 * Invalid UDT field refs
 *
 *     a.b#.c = value
 *     a.b$.c = value
 *     a#.b.c = value
 *     a$.b.c = value
 *     a.b#(x).c = value
 *     a.b$(x).c = value
 *     a#(x).b.c = value
 *     a$(x).b.c = value
 *     func_stmnt().b.c = value
 *     cmd stmnt().b.c = value
 *     func_stmnt#().b.c = value
 *     cmd stmnt#().b.c = value
 *     func_stmnt$().b.c = value
 *     cmd stmnt$().b.c = value
 *
 *     value = a.b#.c
 *     value = a#.b.c
 *     value = a.b$.c
 *     value = a$.b.c
 *     value = a.b#(x).c
 *     value = a.b$(x).c
 *     value = a#(x).b.c
 *     value = a$(x).b.c
 *     value = func_expr#().b.c
 *     value = cmd expr#().b.c
 *     value = func_expr$().b.c
 *     value = cmd expr$().b.c
 *
 *     a.b.c
 *     a(x).b.c
 *     func_stmnt().b.c
 *     cmd stmnt().b.c
 */

TEST_F(NAME, udt_ass_value)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "a.b.c = value", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsTrue());
}

TEST_F(NAME, udt_float_ass_value)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "a.b.c# = value", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::FLOAT, "c"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsTrue());
}

TEST_F(NAME, udt_string_ass_value)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "a.b.c$ = value", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::STRING, "c"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsTrue());
}

TEST_F(NAME, udt_arr_ass_value)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "a.b.c(x) = value", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsTrue());
}

TEST_F(NAME, udt_arr_float_ass_value)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "a.b.c#(x) = value", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::FLOAT, "c"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsTrue());
}

TEST_F(NAME, udt_arr_string_ass_value)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "a.b.c$(x) = value", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::STRING, "c"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsTrue());
}

TEST_F(NAME, udt_inner_arr_ass_value)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "a.b(x).c = value", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsTrue());
}

TEST_F(NAME, udt_outer_arr_ass_value)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "a(x).b.c = value", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsTrue());
}

TEST_F(NAME, value_ass_udt)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "value = a.b.c", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsTrue());
}

TEST_F(NAME, value_ass_udt_float)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "value = a.b.c#", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::FLOAT, "c"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsTrue());
}

TEST_F(NAME, value_ass_udt_string)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "value = a.b.c$", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::STRING, "c"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsTrue());
}

TEST_F(NAME, value_ass_udt_arr)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "value = a.b.c(x)", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "x"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsTrue());
}

TEST_F(NAME, value_ass_udt_arr_float)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "value = a.b.c#(x)", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::FLOAT, "c"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "x"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsTrue());
}

TEST_F(NAME, value_ass_udt_arr_string)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "value = a.b.c$(x)", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::STRING, "c"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "x"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsTrue());
}

TEST_F(NAME, value_ass_udt_inner_arr)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "value = a.b(x).c", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsTrue());
}

TEST_F(NAME, value_ass_udt_outer_arr)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "value = a(x).b.c", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitFuncCallExprOrArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsTrue());
}

TEST_F(NAME, value_ass_func_returning_udt)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "value = func_expr().b.c", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitFuncCallExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "func_expr"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsTrue());
}

TEST_F(NAME, value_ass_command_returning_udt)
{
    using Ann = ast::Symbol::Annotation;

    cmdIndex.addCommand(new cmd::Command(nullptr, "cmd expr", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test", "value = cmd expr().b.c", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitCommandExpr(CommandExprEq("cmd expr"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsTrue());
}

TEST_F(NAME, udt_inner_float_ass_value)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "a.b#.c = value", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::FLOAT, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsFalse());
}

TEST_F(NAME, udt_inner_string_ass_value)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "a.b$.c = value", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::STRING, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsFalse());
}

TEST_F(NAME, udt_outer_float_ass_value)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "a#.b.c = value", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::FLOAT, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsFalse());
}

TEST_F(NAME, udt_outer_string_ass_value)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "a$.b.c = value", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::STRING, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsFalse());
}

TEST_F(NAME, udt_inner_float_arr_ass_value)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "a.b#(x).c = value", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::FLOAT, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsFalse());
}

TEST_F(NAME, udt_inner_string_arr_ass_value)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "a.b$(x).c = value", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::STRING, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsFalse());
}

TEST_F(NAME, udt_outer_float_arr_ass_value)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "a#(x).b.c = value", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::FLOAT, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsFalse());
}

TEST_F(NAME, udt_outer_string_arr_ass_value)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "a$(x).b.c = value", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitUDTFieldAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::STRING, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsFalse());
}

TEST_F(NAME, func_returning_udt_ass_value)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "func_stmnt().b.c = value", matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, command_returning_udt_ass_value)
{
    cmdIndex.addCommand(new cmd::Command(nullptr, "cmd stmnt", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test", "cmd stmnt().b.c = value", matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_func_returning_udt_ass_value)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "func_stmnt#().b.c = value", matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_command_returning_udt_ass_value)
{
    cmdIndex.addCommand(new cmd::Command(nullptr, "cmd stmnt#", "", cmd::Command::Type::Float, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test", "cmd stmnt#().b.c = value", matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_func_returning_udt_ass_value)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "func_stmnt$().b.c = value", matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_command_returning_udt_ass_value)
{
    cmdIndex.addCommand(new cmd::Command(nullptr, "cmd stmnt$", "", cmd::Command::Type::Float, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test", "cmd stmnt$().b.c = value", matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, value_ass_udt_inner_float)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "value = a.b#.c", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::FLOAT, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsFalse());
}

TEST_F(NAME, value_ass_udt_outer_float)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "value = a#.b.c", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::FLOAT, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsFalse());
}

TEST_F(NAME, value_ass_udt_inner_string)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "value = a.b$.c", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::STRING, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsFalse());
}

TEST_F(NAME, value_ass_udt_outer_string)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "value = a$.b.c", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::STRING, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsFalse());
}

TEST_F(NAME, value_ass_udt_inner_float_arr)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "value = a.b#(x).c", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::FLOAT, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsFalse());
}

TEST_F(NAME, value_ass_udt_inner_string_arr)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "value = a.b$(x).c", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::STRING, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsFalse());
}

TEST_F(NAME, value_ass_udt_outer_float_arr)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "value = a#(x).b.c", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitFuncCallExprOrArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::FLOAT, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsFalse());
}

TEST_F(NAME, value_ass_udt_outer_string_arr)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "value = a$(x).b.c", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitFuncCallExprOrArrayRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::STRING, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsFalse());
}

TEST_F(NAME, value_ass_float_func_returning_udt)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "value = func_expr#().b.c", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitFuncCallExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::FLOAT, "func_expr"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsFalse());
}

TEST_F(NAME, value_ass_float_command_returning_udt)
{
    using Ann = ast::Symbol::Annotation;

    cmdIndex.addCommand(new cmd::Command(nullptr, "cmd expr#", "", cmd::Command::Type::Float, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test", "value = cmd expr#().b.c", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitCommandExpr(CommandExprEq("cmd expr#"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsFalse());
}

TEST_F(NAME, value_ass_string_func_returning_udt)
{
    using Ann = ast::Symbol::Annotation;

    ast = driver->parse("test", "value = func_expr$().b.c", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitFuncCallExpr(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::STRING, "func_expr"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsFalse());
}

TEST_F(NAME, value_ass_string_command_returning_udt)
{
    using Ann = ast::Symbol::Annotation;

    cmdIndex.addCommand(new cmd::Command(nullptr, "cmd expr$", "", cmd::Command::Type::Float, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test", "value = cmd expr$().b.c", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "value"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldOuter(_)).After(exp);
    exp = EXPECT_CALL(v, visitCommandExpr(CommandExprEq("cmd expr$"))).After(exp);
    exp = EXPECT_CALL(v, visitUDTFieldInner(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "b"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Ann::NONE, "c"))).After(exp);
    ast->accept(&v);

    astpost::ValidateUDTFieldNames post;
    ASSERT_THAT(post.execute(ast), IsFalse());
}

TEST_F(NAME, udt_cant_be_a_statement)
{
    ast = driver->parse("test", "a.b.c", matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, udt_outer_arr_cant_be_a_statement)
{
    ast = driver->parse("test", "a(x).b.c", matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, func_returning_udt_cant_be_a_statement)
{
    ast = driver->parse("test", "func_stmnt().b.c", matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, command_returning_udt_cant_be_a_statement)
{
    cmdIndex.addCommand(new cmd::Command(nullptr, "cmd stmnt", "", cmd::Command::Type::Float, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test", "cmd stmnt().b.c", matcher);
    ASSERT_THAT(ast, IsNull());
}
