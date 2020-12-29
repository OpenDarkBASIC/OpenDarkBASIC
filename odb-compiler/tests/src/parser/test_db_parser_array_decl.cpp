#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"

#define NAME db_parser_array_decl

using namespace testing;
using namespace odb;

class NAME : public ParserTestHarness
{
public:
};

/*
 * All possible valid array declarations:
 *
 *     dim var(...)
 *     dim var#(...)
 *     dim var$(...)
 *     local dim var(...)
 *     local dim var#(...)
 *     local dim var$(...)
 *     global dim var(...)
 *     global dim var#(...)
 *     global dim var$(...)
 *
 *     dim var(...) as double integer
 *     dim var(...) as integer
 *     dim var(...) as dword
 *     dim var(...) as word
 *     dim var(...) as byte
 *     dim var(...) as boolean
 *     dim var(...) as double float
 *     dim var(...) as float
 *     dim var#(...) as double float
 *     dim var#(...) as float
 *     dim var(...) as string
 *     dim var$(...) as string
 *
 *     local dim var(...) as double integer
 *     local dim var(...) as integer
 *     local dim var(...) as dword
 *     local dim var(...) as word
 *     local dim var(...) as byte
 *     local dim var(...) as boolean
 *     local dim var(...) as double float
 *     local dim var(...) as float
 *     local dim var#(...) as double float
 *     local dim var#(...) as float
 *     local dim var(...) as string
 *     local dim var$(...) as string
 *
 *     global dim var(...) as double integer
 *     global dim var(...) as integer
 *     global dim var(...) as dword
 *     global dim var(...) as word
 *     global dim var(...) as byte
 *     global dim var(...) as boolean
 *     global dim var(...) as double float
 *     global dim var(...) as float
 *     global dim var#(...) as double float
 *     global dim var#(...) as float
 *     global dim var(...) as string
 *     global dim var$(...) as string
 *
 * Invalid variable declarations:
 *
 *     dim var#(...) as double integer
 *     dim var#(...) as integer
 *     dim var#(...) as dword
 *     dim var#(...) as word
 *     dim var#(...) as byte
 *     dim var#(...) as boolean
 *     dim var#(...) as string
 *
 *     dim var$(...) as double integer
 *     dim var$(...) as integer
 *     dim var$(...) as dword
 *     dim var$(...) as word
 *     dim var$(...) as byte
 *     dim var$(...) as boolean
 *     dim var$(...) as double float
 *     dim var$(...) as float
 *
 *     local dim var#(...) as double integer
 *     local dim var#(...) as integer
 *     local dim var#(...) as dword
 *     local dim var#(...) as word
 *     local dim var#(...) as byte
 *     local dim var#(...) as boolean
 *     local dim var#(...) as string
 *
 *     local dim var$(...) as double integer
 *     local dim var$(...) as integer
 *     local dim var$(...) as dword
 *     local dim var$(...) as word
 *     local dim var$(...) as byte
 *     local dim var$(...) as boolean
 *     local dim var$(...) as double float
 *     local dim var$(...) as float
 *
 *     global dim var#(...) as double integer
 *     global dim var#(...) as integer
 *     global dim var#(...) as dword
 *     global dim var#(...) as word
 *     global dim var#(...) as byte
 *     global dim var#(...) as boolean
 *     global dim var#(...) as string
 *
 *     global dim var$(...) as double integer
 *     global dim var$(...) as integer
 *     global dim var$(...) as dword
 *     global dim var$(...) as word
 *     global dim var$(...) as byte
 *     global dim var$(...) as boolean
 *     global dim var$(...) as double float
 *     global dim var$(...) as float
 *
 *     dim var()
 *     dim var#()
 *     dim var$()
 *     local dim var()
 *     local dim var#()
 *     local dim var$()
 *     global dim var()
 *     global dim var#()
 *     global dim var$()
 *
 *     dim var() as double integer
 *     dim var() as integer
 *     dim var() as dword
 *     dim var() as word
 *     dim var() as byte
 *     dim var() as boolean
 *     dim var() as double float
 *     dim var() as float
 *     dim var#() as double float
 *     dim var#() as float
 *     dim var() as string
 *     dim var$() as string
 *
 *     local dim var() as double integer
 *     local dim var() as integer
 *     local dim var() as dword
 *     local dim var() as word
 *     local dim var() as byte
 *     local dim var() as boolean
 *     local dim var() as double float
 *     local dim var() as float
 *     local dim var#() as double float
 *     local dim var#() as float
 *     local dim var() as string
 *     local dim var$() as string
 *
 *     global dim var() as double integer
 *     global dim var() as integer
 *     global dim var() as dword
 *     global dim var() as word
 *     global dim var() as byte
 *     global dim var() as boolean
 *     global dim var() as double float
 *     global dim var() as float
 *     global dim var#() as double float
 *     global dim var#() as float
 *     global dim var() as string
 *     global dim var$() as string
 */

TEST_F(NAME, float_var_cannot_be_declared_as_double_integer)
{
    ast = driver->parseString("test", "dim arr#(2, 3) as double integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_var_cannot_be_declared_as_integer)
{
    ast = driver->parseString("test", "dim arr#(2, 3) as integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_var_cannot_be_declared_as_dword)
{
    ast = driver->parseString("test", "dim arr#(2, 3) as dword");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_var_cannot_be_declared_as_word)
{
    ast = driver->parseString("test", "dim arr#(2, 3) as word");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_var_cannot_be_declared_as_byte)
{
    ast = driver->parseString("test", "dim arr#(2, 3) as byte");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_var_cannot_be_declared_as_boolean)
{
    ast = driver->parseString("test", "dim arr#(2, 3) as boolean");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, float_var_cannot_be_declared_as_string)
{
    ast = driver->parseString("test", "dim arr#(2, 3) as string");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_var_cannot_be_declared_as_double_integer)
{
    ast = driver->parseString("test", "dim arr$(2, 3) as double integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_var_cannot_be_declared_as_integer)
{
    ast = driver->parseString("test", "dim arr$(2, 3) as integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_var_cannot_be_declared_as_dword)
{
    ast = driver->parseString("test", "dim arr$(2, 3) as dword");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_var_cannot_be_declared_as_word)
{
    ast = driver->parseString("test", "dim arr$(2, 3) as word");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_var_cannot_be_declared_as_byte)
{
    ast = driver->parseString("test", "dim arr$(2, 3) as byte");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_var_cannot_be_declared_as_boolean)
{
    ast = driver->parseString("test", "dim arr$(2, 3) as boolean");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_var_cannot_be_declared_as_double_float)
{
    ast = driver->parseString("test", "dim arr$(2, 3) as double float");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, string_var_cannot_be_declared_as_float)
{
    ast = driver->parseString("test", "dim arr$(2, 3) as float");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_float_var_cannot_be_declared_as_double_integer)
{
    ast = driver->parseString("test", "local dim arr#(2, 3) as double integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_float_var_cannot_be_declared_as_integer)
{
    ast = driver->parseString("test", "local dim arr#(2, 3) as integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_float_var_cannot_be_declared_as_dword)
{
    ast = driver->parseString("test", "local dim arr#(2, 3) as dword");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_float_var_cannot_be_declared_as_word)
{
    ast = driver->parseString("test", "local dim arr#(2, 3) as word");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_float_var_cannot_be_declared_as_byte)
{
    ast = driver->parseString("test", "local dim arr#(2, 3) as byte");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_float_var_cannot_be_declared_as_boolean)
{
    ast = driver->parseString("test", "local dim arr#(2, 3) as boolean");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_float_var_cannot_be_declared_as_string)
{
    ast = driver->parseString("test", "local dim arr#(2, 3) as string");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_string_var_cannot_be_declared_as_double_integer)
{
    ast = driver->parseString("test", "local dim arr$(2, 3) as double integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_string_var_cannot_be_declared_as_integer)
{
    ast = driver->parseString("test", "local dim arr$(2, 3) as integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_string_var_cannot_be_declared_as_dword)
{
    ast = driver->parseString("test", "local dim arr$(2, 3) as dword");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_string_var_cannot_be_declared_as_word)
{
    ast = driver->parseString("test", "local dim arr$(2, 3) as word");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_string_var_cannot_be_declared_as_byte)
{
    ast = driver->parseString("test", "local dim arr$(2, 3) as byte");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_string_var_cannot_be_declared_as_boolean)
{
    ast = driver->parseString("test", "local dim arr$(2, 3) as boolean");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_string_var_cannot_be_declared_as_double_float)
{
    ast = driver->parseString("test", "local dim arr$(2, 3) as double float");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, local_string_var_cannot_be_declared_as_float)
{
    ast = driver->parseString("test", "local dim arr$(2, 3) as float");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_float_var_cannot_be_declared_as_double_integer)
{
    ast = driver->parseString("test", "global dim arr#(2, 3) as double integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_float_var_cannot_be_declared_as_integer)
{
    ast = driver->parseString("test", "global dim arr#(2, 3) as integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_float_var_cannot_be_declared_as_dword)
{
    ast = driver->parseString("test", "global dim arr#(2, 3) as dword");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_float_var_cannot_be_declared_as_word)
{
    ast = driver->parseString("test", "global dim arr#(2, 3) as word");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_float_var_cannot_be_declared_as_byte)
{
    ast = driver->parseString("test", "global dim arr#(2, 3) as byte");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_float_var_cannot_be_declared_as_boolean)
{
    ast = driver->parseString("test", "global dim arr#(2, 3) as boolean");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_float_var_cannot_be_declared_as_string)
{
    ast = driver->parseString("test", "global dim arr#(2, 3) as string");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_string_var_cannot_be_declared_as_double_integer)
{
    ast = driver->parseString("test", "global dim arr$(2, 3) as double integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_string_var_cannot_be_declared_as_integer)
{
    ast = driver->parseString("test", "global dim arr$(2, 3) as integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_string_var_cannot_be_declared_as_dword)
{
    ast = driver->parseString("test", "global dim arr$(2, 3) as dword");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_string_var_cannot_be_declared_as_word)
{
    ast = driver->parseString("test", "global dim arr$(2, 3) as word");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_string_var_cannot_be_declared_as_byte)
{
    ast = driver->parseString("test", "global dim arr$(2, 3) as byte");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_string_var_cannot_be_declared_as_boolean)
{
    ast = driver->parseString("test", "global dim arr$(2, 3) as boolean");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_string_var_cannot_be_declared_as_double_float)
{
    ast = driver->parseString("test", "global dim arr$(2, 3) as double float");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, global_string_var_cannot_be_declared_as_float)
{
    ast = driver->parseString("test", "global dim arr$(2, 3) as float");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_array_decl_defaults_to_local_and_integer)
{
    ast = driver->parseString("test", "dim arr()");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_array_float_decl_defaults_to_local_and_float)
{
    ast = driver->parseString("test", "dim arr#()");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_array_string_decl_defaults_to_local_and_string)
{
    ast = driver->parseString("test", "dim arr$()");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_local_array_decl_defaults_to_integer)
{
    ast = driver->parseString("test", "local dim arr()");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_local_array_float_decl_defaults_to_float)
{
    ast = driver->parseString("test", "local dim arr#()");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_local_array_string_decl_defaults_to_string)
{
    ast = driver->parseString("test", "local dim arr$()");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_global_array_decl_defaults_to_integer)
{
    ast = driver->parseString("test", "global dim arr()");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_global_array_float_decl_defaults_to_float)
{
    ast = driver->parseString("test", "global dim arr#()");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_global_array_string_decl_defaults_to_string)
{
    ast = driver->parseString("test", "global dim arr$()");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_array_as_double_integer)
{
    ast = driver->parseString("test", "dim arr() as double integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_array_as_integer)
{
    ast = driver->parseString("test", "dim arr() as integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_array_as_dword)
{
    ast = driver->parseString("test", "dim arr() as dword");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_array_as_word)
{
    ast = driver->parseString("test", "dim arr() as word");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_array_as_byte)
{
    ast = driver->parseString("test", "dim arr() as byte");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_array_as_boolean)
{
    ast = driver->parseString("test", "dim arr() as boolean");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_array_as_double_float)
{
    ast = driver->parseString("test", "dim arr() as double float");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_array_as_float)
{
    ast = driver->parseString("test", "dim arr() as float");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_float_array_as_double_float)
{
    ast = driver->parseString("test", "dim arr#() as double float");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_float_array_as_float)
{
    ast = driver->parseString("test", "dim arr#() as float");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_array_as_string)
{
    ast = driver->parseString("test", "dim arr() as string");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_string_array_as_string)
{
    ast = driver->parseString("test", "dim arr$() as string");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_local_array_as_double_integer)
{
    ast = driver->parseString("test", "local dim arr() as double integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_local_array_as_integer)
{
    ast = driver->parseString("test", "local dim arr() as integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_local_array_as_dword)
{
    ast = driver->parseString("test", "local dim arr() as dword");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_local_array_as_word)
{
    ast = driver->parseString("test", "local dim arr() as word");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_local_array_as_byte)
{
    ast = driver->parseString("test", "local dim arr() as byte");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_local_array_as_boolean)
{
    ast = driver->parseString("test", "local dim arr() as boolean");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_local_array_as_double_float)
{
    ast = driver->parseString("test", "local dim arr() as double float");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_local_array_as_float)
{
    ast = driver->parseString("test", "local dim arr() as float");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_float_local_array_as_double_float)
{
    ast = driver->parseString("test", "local dim arr#() as double float");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_float_local_array_as_float)
{
    ast = driver->parseString("test", "local dim arr#() as float");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_local_array_as_string)
{
    ast = driver->parseString("test", "local dim arr() as string");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_string_local_array_as_string)
{
    ast = driver->parseString("test", "local dim arr$() as string");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_global_array_as_double_integer)
{
    ast = driver->parseString("test", "global dim arr() as double integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_global_array_as_integer)
{
    ast = driver->parseString("test", "global dim arr() as integer");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_global_array_as_dword)
{
    ast = driver->parseString("test", "global dim arr() as dword");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_global_array_as_word)
{
    ast = driver->parseString("test", "global dim arr() as word");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_global_array_as_byte)
{
    ast = driver->parseString("test", "global dim arr() as byte");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_global_array_as_boolean)
{
    ast = driver->parseString("test", "global dim arr() as boolean");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_global_array_as_double_float)
{
    ast = driver->parseString("test", "global dim arr() as double float");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_global_array_as_float)
{
    ast = driver->parseString("test", "global dim arr() as float");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_float_global_array_as_double_float)
{
    ast = driver->parseString("test", "global dim arr#() as double float");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_float_global_array_as_float)
{
    ast = driver->parseString("test", "global dim arr#() as float");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_global_array_as_string)
{
    ast = driver->parseString("test", "global dim arr() as string");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, empty_string_global_array_as_string)
{
    ast = driver->parseString("test", "global dim arr$() as string");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, array_decl_defaults_to_local_and_integer)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "dim arr(2, 3)");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitIntegerArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, array_float_decl_defaults_to_local_and_float)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "dim arr#(2, 3)");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFloatArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::FLOAT, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, array_string_decl_defaults_to_local_and_string)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "dim arr$(2, 3)");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitStringArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::STRING, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_array_decl_defaults_to_integer)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local dim arr(2, 3)");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitIntegerArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_array_float_decl_defaults_to_float)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local dim arr#(2, 3)");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFloatArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::FLOAT, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_array_string_decl_defaults_to_string)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local dim arr$(2, 3)");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitStringArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::STRING, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_array_decl_defaults_to_integer)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global dim arr(2, 3)");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitIntegerArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_array_float_decl_defaults_to_float)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global dim arr#(2, 3)");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFloatArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::FLOAT, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_array_string_decl_defaults_to_string)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global dim arr$(2, 3)");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitStringArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::STRING, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, array_as_double_integer)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "dim arr(2, 3) as double integer");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleIntegerArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, array_as_integer)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "dim arr(2, 3) as integer");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitIntegerArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, array_as_dword)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "dim arr(2, 3) as dword");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDwordArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, array_as_word)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "dim arr(2, 3) as word");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitWordArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, array_as_byte)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "dim arr(2, 3) as byte");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitByteArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, array_as_boolean)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "dim arr(2, 3) as boolean");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitBooleanArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, array_as_double_float)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "dim arr(2, 3) as double float");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleFloatArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, array_as_float)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "dim arr(2, 3) as float");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFloatArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, float_array_as_double_float)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "dim arr#(2, 3) as double float");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleFloatArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::FLOAT, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, float_array_as_float)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "dim arr#(2, 3) as float");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFloatArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::FLOAT, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, array_as_string)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "dim arr(2, 3) as string");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitStringArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, string_array_as_string)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "dim arr$(2, 3) as string");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitStringArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::STRING, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_array_as_double_integer)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local dim arr(2, 3) as double integer");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleIntegerArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_array_as_integer)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local dim arr(2, 3) as integer");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitIntegerArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_array_as_dword)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local dim arr(2, 3) as dword");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDwordArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_array_as_word)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local dim arr(2, 3) as word");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitWordArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_array_as_byte)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local dim arr(2, 3) as byte");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitByteArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_array_as_boolean)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local dim arr(2, 3) as boolean");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitBooleanArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_array_as_double_float)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local dim arr(2, 3) as double float");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleFloatArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_array_as_float)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local dim arr(2, 3) as float");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFloatArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, float_local_array_as_double_float)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local dim arr#(2, 3) as double float");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleFloatArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::FLOAT, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, float_local_array_as_float)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local dim arr#(2, 3) as float");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFloatArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::FLOAT, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, local_array_as_string)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local dim arr(2, 3) as string");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitStringArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, string_local_array_as_string)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "local dim arr$(2, 3) as string");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitStringArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::LOCAL, Annotation::STRING, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_array_as_double_integer)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global dim arr(2, 3) as double integer");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleIntegerArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_array_as_integer)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global dim arr(2, 3) as integer");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitIntegerArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_array_as_dword)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global dim arr(2, 3) as dword");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDwordArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_array_as_word)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global dim arr(2, 3) as word");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitWordArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_array_as_byte)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global dim arr(2, 3) as byte");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitByteArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_array_as_boolean)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global dim arr(2, 3) as boolean");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitBooleanArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_array_as_double_float)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global dim arr(2, 3) as double float");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleFloatArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_array_as_float)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global dim arr(2, 3) as float");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFloatArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, float_global_array_as_double_float)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global dim arr#(2, 3) as double float");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitDoubleFloatArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::FLOAT, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, float_global_array_as_float)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global dim arr#(2, 3) as float");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFloatArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::FLOAT, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, global_array_as_string)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global dim arr(2, 3) as string");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitStringArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::NONE, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, string_global_array_as_string)
{
    using Annotation = ast::Symbol::Annotation;
    using Scope = ast::Symbol::Scope;

    ast = driver->parseString("test", "global dim arr$(2, 3) as string");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitStringArrayDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(ScopedAnnotatedSymbolEq(Scope::GLOBAL, Annotation::STRING, "arr"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);

    ast->accept(&v);
}
