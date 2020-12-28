#include <gmock/gmock.h>
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/commands/Command.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME db_parser_var_decl

using namespace testing;
using namespace odb;

class NAME : public ParserTestHarness
{
public:
};

/*
 * All possible valid variable declarations:
 *
 *     local var
 *     local var#
 *     local var$
 *     global var
 *     global var#
 *     global var$
 *
 *     local var = x
 *     local var# = x
 *     local var$ = x
 *     global var = x
 *     global var# = x
 *     global var$ = x
 *
 *     var as double integer
 *     var as integer
 *     var as dword
 *     var as word
 *     var as byte
 *     var as boolean
 *     var as double float
 *     var as float
 *     var# as double float
 *     var# as float
 *     var as string
 *     var$ as string
 *
 *     local var as double integer
 *     local var as integer
 *     local var as dword
 *     local var as word
 *     local var as byte
 *     local var as boolean
 *     local var as double float
 *     local var as float
 *     local var# as double float
 *     local var# as float
 *     local var as string
 *     local var$ as string
 *
 *     global var as double integer
 *     global var as integer
 *     global var as dword
 *     global var as word
 *     global var as byte
 *     global var as boolean
 *     global var as double float
 *     global var as float
 *     global var# as double float
 *     global var# as float
 *     global var as string
 *     global var$ as string
 *
 *     var as double integer = x
 *     var as integer = x
 *     var as dword = x
 *     var as word = x
 *     var as byte = x
 *     var as boolean = x
 *     var as double float = x
 *     var as float = x
 *     var# as double float = x
 *     var# as float = x
 *     var as string = x
 *     var$ as string = x
 *
 *     local var as double integer = x
 *     local var as integer = x
 *     local var as dword = x
 *     local var as word = x
 *     local var as byte = x
 *     local var as boolean = x
 *     local var as double float = x
 *     local var as float = x
 *     local var# as double float = x
 *     local var# as float = x
 *     local var as string = x
 *     local var$ as string = x
 *
 *     global var as double integer = x
 *     global var as integer = x
 *     global var as dword = x
 *     global var as word = x
 *     global var as byte = x
 *     global var as boolean = x
 *     global var as double float = x
 *     global var as float = x
 *     global var# as double float = x
 *     global var# as float = x
 *     global var as string = x
 *     global var$ as string = x
 *
 * Invalid variable declarations:
 *
 *     var
 *     var#
 *     var$
 *
 *     var# as double integer
 *     var# as integer
 *     var# as dword
 *     var# as word
 *     var# as byte
 *     var# as boolean
 *     var# as string
 *
 *     var$ as double integer
 *     var$ as integer
 *     var$ as dword
 *     var$ as word
 *     var$ as byte
 *     var$ as boolean
 *     var$ as double float
 *     var$ as float
 *
 *     local var# as double integer
 *     local var# as integer
 *     local var# as dword
 *     local var# as word
 *     local var# as byte
 *     local var# as boolean
 *     local var# as string
 *
 *     local var$ as double integer
 *     local var$ as integer
 *     local var$ as dword
 *     local var$ as word
 *     local var$ as byte
 *     local var$ as boolean
 *     local var$ as double float
 *     local var$ as float
 *
 *     global var# as double integer
 *     global var# as integer
 *     global var# as dword
 *     global var# as word
 *     global var# as byte
 *     global var# as boolean
 *     global var# as string
 *
 *     global var$ as double integer
 *     global var$ as integer
 *     global var$ as dword
 *     global var$ as word
 *     global var$ as byte
 *     global var$ as boolean
 *     global var$ as double float
 *     global var$ as float
 */

TEST_F(NAME, variable_alone_is_not_a_valid_statement)
{
    ast = driver->parseString("test", "var");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_variable_alone_is_not_a_valid_statement)
{
    ast = driver->parseString("test", "var#");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_variable_alone_is_not_a_valid_statement)
{
    ast = driver->parseString("test", "var$");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_variable_cannot_be_double_integer)
{
    ast = driver->parseString("test", "var# as double integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_variable_cannot_be_integer)
{
    ast = driver->parseString("test", "var# as integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_variable_cannot_be_dword)
{
    ast = driver->parseString("test", "var# as dword");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_variable_cannot_be_word)
{
    ast = driver->parseString("test", "var# as word");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_variable_cannot_be_byte)
{
    ast = driver->parseString("test", "var# as byte");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_variable_cannot_be_boolean)
{
    ast = driver->parseString("test", "var# as boolean");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_variable_cannot_be_string)
{
    ast = driver->parseString("test", "var# as string");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_variable_cannot_be_double_integer)
{
    ast = driver->parseString("test", "var$ as double integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_variable_cannot_be_integer)
{
    ast = driver->parseString("test", "var$ as integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_variable_cannot_be_dword)
{
    ast = driver->parseString("test", "var$ as dword");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_variable_cannot_be_word)
{
    ast = driver->parseString("test", "var$ as word");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_variable_cannot_be_byte)
{
    ast = driver->parseString("test", "var$ as byte");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_variable_cannot_be_boolean)
{
    ast = driver->parseString("test", "var$ as boolean");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_variable_cannot_be_double_float)
{
    ast = driver->parseString("test", "var$ as double float");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_variable_cannot_be_float)
{
    ast = driver->parseString("test", "var$ as float");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_float_variable_cannot_be_double_integer)
{
    ast = driver->parseString("test", "local var# as double integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_float_variable_cannot_be_integer)
{
    ast = driver->parseString("test", "local var# as integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_float_variable_cannot_be_dword)
{
    ast = driver->parseString("test", "local var# as dword");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_float_variable_cannot_be_word)
{
    ast = driver->parseString("test", "local var# as word");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_float_variable_cannot_be_byte)
{
    ast = driver->parseString("test", "local var# as byte");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_float_variable_cannot_be_boolean)
{
    ast = driver->parseString("test", "local var# as boolean");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_float_variable_cannot_be_string)
{
    ast = driver->parseString("test", "local var# as string");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_string_variable_cannot_be_double_integer)
{
    ast = driver->parseString("test", "local var$ as double integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_string_variable_cannot_be_integer)
{
    ast = driver->parseString("test", "local var$ as integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_string_variable_cannot_be_dword)
{
    ast = driver->parseString("test", "local var$ as dword");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_string_variable_cannot_be_word)
{
    ast = driver->parseString("test", "local var$ as word");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_string_variable_cannot_be_byte)
{
    ast = driver->parseString("test", "local var$ as byte");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_string_variable_cannot_be_boolean)
{
    ast = driver->parseString("test", "local var$ as boolean");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_string_variable_cannot_be_double_float)
{
    ast = driver->parseString("test", "local var$ as double float");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_string_variable_cannot_be_float)
{
    ast = driver->parseString("test", "local var$ as float");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_float_variable_cannot_be_double_integer)
{
    ast = driver->parseString("test", "global var# as double integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_float_variable_cannot_be_integer)
{
    ast = driver->parseString("test", "global var# as integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_float_variable_cannot_be_dword)
{
    ast = driver->parseString("test", "global var# as dword");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_float_variable_cannot_be_word)
{
    ast = driver->parseString("test", "global var# as word");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_float_variable_cannot_be_byte)
{
    ast = driver->parseString("test", "global var# as byte");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_float_variable_cannot_be_boolean)
{
    ast = driver->parseString("test", "global var# as boolean");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_float_variable_cannot_be_string)
{
    ast = driver->parseString("test", "global var# as string");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_string_variable_cannot_be_double_integer)
{
    ast = driver->parseString("test", "global var$ as double integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_string_variable_cannot_be_integer)
{
    ast = driver->parseString("test", "global var$ as integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_string_variable_cannot_be_dword)
{
    ast = driver->parseString("test", "global var$ as dword");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_string_variable_cannot_be_word)
{
    ast = driver->parseString("test", "global var$ as word");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_string_variable_cannot_be_byte)
{
    ast = driver->parseString("test", "global var$ as byte");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_string_variable_cannot_be_boolean)
{
    ast = driver->parseString("test", "global var$ as boolean");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_string_variable_cannot_be_double_float)
{
    ast = driver->parseString("test", "global var$ as double float");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_string_variable_cannot_be_float)
{
    ast = driver->parseString("test", "global var$ as float");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, disallow_variable_decl_that_is_a_keyword_1)
{
    ast = driver->parseString("test", "global byte as byte");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, disallow_variable_decl_that_is_a_keyword_2)
{
    ast = driver->parseString("test", "byte as byte");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, disallow_variable_decl_that_is_a_keyword_3)
{
    ast = driver->parseString("test", "local integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_var_decl_defaults_to_integer)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitIntegerVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitIntegerLiteral(IntegerLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_var_decl_defaults_integer)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitIntegerVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitIntegerLiteral(IntegerLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_float_var_decl_has_type_float)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var#");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::FLOAT, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_float_var_decl_has_type_float)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var#");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::FLOAT, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_string_var_decl_has_type_string)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var$");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitStringVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::STRING, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitStringLiteral(StringLiteralEq(""))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_string_var_decl_has_type_string)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var$");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitStringVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::STRING, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitStringLiteral(StringLiteralEq(""))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_var_with_assignment_defaults_to_integer)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitIntegerVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_var_with_assignment_defaults_to_integer)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitIntegerVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_float_var_with_assignment_has_type_float)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var# = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::FLOAT, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_float_var_with_assignment_has_type_float)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var# = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::FLOAT, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_string_var_with_assignment_has_type_string)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var$ = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitStringVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::STRING, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_string_var_with_assignment_has_type_string)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var$ = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitStringVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::STRING, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_as_double_integer)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var as double integer");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleIntegerVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleIntegerLiteral(DoubleIntegerLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_as_integer)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var as integer");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitIntegerVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitIntegerLiteral(IntegerLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_as_dword)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var as dword");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDwordVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDwordLiteral(DwordLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_as_word)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var as word");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitWordVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitWordLiteral(WordLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_as_byte)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var as byte");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitByteVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_as_boolean)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var as boolean");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitBooleanVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitBooleanLiteral(BooleanLiteralEq(false))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_as_double_float)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var as double float");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(0.0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_as_float)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var as float");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(0.0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_as_double_float_annotated)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var# as double float");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::FLOAT, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(0.0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_as_float_annotated)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var# as float");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::FLOAT, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(0.0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_as_string)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var as string");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitStringVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitStringLiteral(StringLiteralEq(""))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_as_string_annotated)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var$ as string");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitStringVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::STRING, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitStringLiteral(StringLiteralEq(""))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_var_as_double_integer)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var as double integer");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleIntegerVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleIntegerLiteral(DoubleIntegerLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_var_as_integer)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var as integer");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitIntegerVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitIntegerLiteral(IntegerLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_var_as_dword)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var as dword");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDwordVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDwordLiteral(DwordLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_var_as_word)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var as word");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitWordVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitWordLiteral(WordLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_var_as_byte)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var as byte");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitByteVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_var_as_boolean)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var as boolean");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitBooleanVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitBooleanLiteral(BooleanLiteralEq(false))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_var_as_double_float)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var as double float");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(0.0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_var_as_float)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var as float");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(0.0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_var_as_double_float_annotated)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var# as double float");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::FLOAT, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(0.0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_var_as_float_annotated)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var# as float");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::FLOAT, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(0.0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_var_as_string)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var as string");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitStringVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitStringLiteral(StringLiteralEq(""))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_var_as_string_annotated)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var$ as string");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitStringVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::STRING, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitStringLiteral(StringLiteralEq(""))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_var_as_double_integer)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var as double integer");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleIntegerVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleIntegerLiteral(DoubleIntegerLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_var_as_integer)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var as integer");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitIntegerVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitIntegerLiteral(IntegerLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_var_as_dword)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var as dword");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDwordVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDwordLiteral(DwordLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_var_as_word)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var as word");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitWordVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitWordLiteral(WordLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_var_as_byte)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var as byte");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitByteVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_var_as_boolean)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var as boolean");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitBooleanVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitBooleanLiteral(BooleanLiteralEq(false))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_var_as_double_float)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var as double float");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(0.0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_var_as_float)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var as float");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(0.0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_var_as_double_float_annotated)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var# as double float");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::FLOAT, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(0.0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_var_as_float_annotated)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var# as float");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::FLOAT, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitFloatLiteral(FloatLiteralEq(0.0))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_var_as_string)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var as string");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitStringVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitStringLiteral(StringLiteralEq(""))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_var_as_string_annotated)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var$ as string");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitStringVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::STRING, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitStringLiteral(StringLiteralEq(""))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_as_double_integer_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var as double integer = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleIntegerVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_as_integer_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var as integer = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitIntegerVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_as_dword_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var as dword = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDwordVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_as_word_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var as word = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitWordVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_as_byte_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var as byte = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitByteVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_as_boolean_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var as boolean = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitBooleanVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_as_double_float_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var as double float = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_as_float_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var as float = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_as_double_float_annotated_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var# as double float = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::FLOAT, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_as_float_annotated_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var# as float = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::FLOAT, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_as_string_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var as string = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitStringVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, var_as_string_annotated_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var$ as string = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitStringVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::STRING, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_var_as_double_integer_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var as double integer = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleIntegerVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_var_as_integer_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var as integer = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitIntegerVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_var_as_dword_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var as dword = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDwordVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_var_as_word_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var as word = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitWordVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_var_as_byte_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var as byte = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitByteVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_var_as_boolean_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var as boolean = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitBooleanVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_var_as_double_float_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var as double float = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_var_as_float_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var as float = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_var_as_double_float_annotated_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var# as double float = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::FLOAT, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_var_as_float_annotated_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var# as float = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::FLOAT, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_var_as_string_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var as string = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitStringVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_var_as_string_annotated_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local var$ as string = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitStringVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::STRING, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_var_as_double_integer_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var as double integer = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleIntegerVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_var_as_integer_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var as integer = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitIntegerVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_var_as_dword_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var as dword = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDwordVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_var_as_word_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var as word = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitWordVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_var_as_byte_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "var as byte = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitByteVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_var_as_boolean_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var as boolean = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitBooleanVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_var_as_double_float_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var as double float = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_var_as_float_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var as float = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_var_as_double_float_annotated_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var# as double float = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::FLOAT, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_var_as_float_annotated_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var# as float = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFloatVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::FLOAT, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_var_as_string_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var as string = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitStringVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_var_as_string_annotated_with_initial_value)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global var$ as string = 5.4");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitStringVarDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(
        ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::STRING, "var"))).After(exp);
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);

    ast->accept(&v);
}
