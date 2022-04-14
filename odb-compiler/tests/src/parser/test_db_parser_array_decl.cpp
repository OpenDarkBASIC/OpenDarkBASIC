#include "odb-compiler/ast/Annotation.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/Scope.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/matchers/ArgListCountEq.hpp"
#include "odb-compiler/tests/matchers/ArrayDeclEq.hpp"
#include "odb-compiler/tests/matchers/BlockStmntCountEq.hpp"
#include "odb-compiler/tests/matchers/LiteralEq.hpp"
#include "odb-compiler/tests/matchers/ScopedAnnotatedSymbolEq.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"

#define NAME db_parser_array_decl

using namespace testing;
using namespace odb;
using namespace ast;

class NAME : public ParserTestHarness
{
public:
};

// Scope specifiers
#define global_str "global"
#define local_str "local"
#define none_str ""

// Scope enums
#define global_scope Scope::GLOBAL
#define local_scope Scope::LOCAL
#define none_scope Scope::LOCAL

// Anotations
#define amp_str "&"
#define percent_str "%"
#define excl_str "!"
#define hash_str "#"
#define dollar_str "$"

// Annotation enums
#define amp_ann Annotation::DOUBLE_INTEGER
#define percent_ann Annotation::WORD
#define hash_ann Annotation::FLOAT
#define excl_ann Annotation::DOUBLE_FLOAT
#define dollar_ann Annotation::STRING
#define none_ann Annotation::NONE

// Type specifiers
#define double_integer_str "double integer"
#define integer_str "integer"
#define dword_str "dword"
#define word_str "word"
#define byte_str "byte"
#define boolean_str "boolean"
#define double_float_str "double float"
#define float_str "float"
#define string_str "string"

// Type initial values
#define double_integer_initial_value 0
#define integer_initial_value 0
#define dword_initial_value 0
#define word_initial_value 0
#define byte_initial_value 0
#define boolean_initial_value false
#define double_float_initial_value 0.0
#define float_initial_value 0.0f
#define string_initial_value ""

// Type names
#define double_integer_type DoubleInteger
#define integer_type Integer
#define dword_type Dword
#define word_type Word
#define byte_type Byte
#define boolean_type Boolean
#define double_float_type DoubleFloat
#define float_type Float
#define string_type String

/*
 * All possible valid array declarations:
 *
 *     dim var(...)
 *     dim var&(...)
 *     dim var%(...)
 *     dim var!(...)
 *     dim var#(...)
 *     dim var$(...)
 *     local dim var(...)
 *     local dim var&(...)
 *     local dim var%(...)
 *     local dim var!(...)
 *     local dim var#(...)
 *     local dim var$(...)
 *     global dim var(...)
 *     global dim var&(...)
 *     global dim var%(...)
 *     global dim var!(...)
 *     global dim var#(...)
 *     global dim var$(...)
 */
#define VALID(scope, ann, type)                                               \
TEST_F(NAME, scope##_dim_arr_##ann##_defaults_to_##type)                      \
{                                                                             \
    ast = driver->parse("test", scope##_str " dim arr" ann##_str "(2, 3)", matcher);\
    ASSERT_THAT(ast, NotNull());                                              \
                                                                              \
    StrictMock<ASTMockVisitor> v;                                             \
    Expectation exp;                                                          \
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));                   \
    exp = EXPECT_CALL(v, visitArrayDecl(ArrayDeclEq(BuiltinType::type##_type))).After(exp);\
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(                          \
        ScopedAnnotatedSymbolEq(scope##_scope, ann##_ann, "arr"))).After(exp);\
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(2))).After(exp);\
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);      \
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);      \
                                                                              \
    ast->accept(&v);                                                          \
}
#define VALID_ALL_SCOPES(ann, type)                                           \
    VALID(none, ann, type)                                                    \
    VALID(local, ann, type)                                                   \
    VALID(global, ann, type)
VALID_ALL_SCOPES(none, integer)
VALID_ALL_SCOPES(amp, double_integer)
VALID_ALL_SCOPES(percent, word)
VALID_ALL_SCOPES(excl, double_float)
VALID_ALL_SCOPES(hash, float)
VALID_ALL_SCOPES(dollar, string)
/*
 *     dim var(...) as double integer
 *     dim var(...) as integer
 *     dim var(...) as dword
 *     dim var(...) as word
 *     dim var(...) as byte
 *     dim var(...) as boolean
 *     dim var(...) as double float
 *     dim var(...) as float
 *     dim var(...) as string
 *     dim var&(...) as double_integer
 *     dim var%(...) as word
 *     dim var!(...) as double float
 *     dim var#(...) as float
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
 *     local dim var(...) as string
 *     local dim var&(...) as double integer
 *     local dim var%(...) as word
 *     local dim var!(...) as double float
 *     local dim var#(...) as float
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
 *     global dim var(...) as string
 *     global dim var&(...) as double integer
 *     global dim var%(...) as word
 *     global dim var!(...) as double float
 *     global dim var#(...) as float
 *     global dim var$(...) as string
 */
#define VALID_AS_TYPE(scope, ann, type)                                       \
TEST_F(NAME, scope##_dim_arr_##ann##_as_##type)                               \
{                                                                             \
    ast = driver->parse("test", scope##_str " dim arr" ann##_str "(2, 3) as " type##_str, matcher);\
    ASSERT_THAT(ast, NotNull());                                              \
                                                                              \
    StrictMock<ASTMockVisitor> v;                                             \
    Expectation exp;                                                          \
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));                   \
    exp = EXPECT_CALL(v, visitArrayDecl(ArrayDeclEq(BuiltinType::type##_type))).After(exp);\
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(                          \
        ScopedAnnotatedSymbolEq(scope##_scope, ann##_ann, "arr"))).After(exp);\
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(2))).After(exp);\
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);      \
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);      \
                                                                              \
    ast->accept(&v);                                                          \
}
#define VALID_AS_TYPE_ALL_SCOPES(ann, type)                                   \
    VALID_AS_TYPE(none, ann, type)                                            \
    VALID_AS_TYPE(local, ann, type)                                           \
    VALID_AS_TYPE(global, ann, type)
VALID_AS_TYPE_ALL_SCOPES(none, double_integer)
VALID_AS_TYPE_ALL_SCOPES(none, integer)
VALID_AS_TYPE_ALL_SCOPES(none, dword)
VALID_AS_TYPE_ALL_SCOPES(none, word)
VALID_AS_TYPE_ALL_SCOPES(none, byte)
VALID_AS_TYPE_ALL_SCOPES(none, boolean)
VALID_AS_TYPE_ALL_SCOPES(none, double_float)
VALID_AS_TYPE_ALL_SCOPES(none, float)
VALID_AS_TYPE_ALL_SCOPES(none, string)
VALID_AS_TYPE_ALL_SCOPES(amp, double_integer)
VALID_AS_TYPE_ALL_SCOPES(percent, word)
VALID_AS_TYPE_ALL_SCOPES(excl, double_float)
VALID_AS_TYPE_ALL_SCOPES(hash, float)
VALID_AS_TYPE_ALL_SCOPES(dollar, string)
/*
 * Invalid variable declarations:
 *
 *     dim var&(...) as integer
 *     dim var&(...) as dword
 *     dim var&(...) as word
 *     dim var&(...) as byte
 *     dim var&(...) as boolean
 *     dim var&(...) as double float
 *     dim var&(...) as float
 *     dim var&(...) as string
 *
 *     local dim var&(...) as integer
 *     local dim var&(...) as dword
 *     local dim var&(...) as word
 *     local dim var&(...) as byte
 *     local dim var&(...) as boolean
 *     local dim var&(...) as double float
 *     local dim var&(...) as float
 *     local dim var&(...) as string
 *
 *     global dim var&(...) as integer
 *     global dim var&(...) as dword
 *     global dim var&(...) as word
 *     global dim var&(...) as byte
 *     global dim var&(...) as boolean
 *     global dim var&(...) as double float
 *     global dim var&(...) as float
 *     global dim var&(...) as string
 */
#define INVALID_AS_TYPE(scope, ann, type)                                     \
TEST_F(NAME, scope##_dim_arr_##ann##_as_##type##_is_invalid)                  \
{                                                                             \
    ast = driver->parse("test", scope##_str " dim arr" ann##_str "(2, 3) as " type##_str, matcher);\
    ASSERT_THAT(ast, IsNull());                                               \
}
#define INVALID_AS_TYPE_ALL_SCOPES(ann, type)                                 \
    INVALID_AS_TYPE(none, ann, type)                                          \
    INVALID_AS_TYPE(local, ann, type)                                         \
    INVALID_AS_TYPE(global, ann, type)
INVALID_AS_TYPE_ALL_SCOPES(amp, integer);
INVALID_AS_TYPE_ALL_SCOPES(amp, dword);
INVALID_AS_TYPE_ALL_SCOPES(amp, word);
INVALID_AS_TYPE_ALL_SCOPES(amp, byte);
INVALID_AS_TYPE_ALL_SCOPES(amp, boolean);
INVALID_AS_TYPE_ALL_SCOPES(amp, double_float);
INVALID_AS_TYPE_ALL_SCOPES(amp, float);
INVALID_AS_TYPE_ALL_SCOPES(amp, string);
/*
 *
 *     dim var%(...) as double integer
 *     dim var%(...) as integer
 *     dim var%(...) as dword
 *     dim var%(...) as byte
 *     dim var%(...) as boolean
 *     dim var%(...) as double float
 *     dim var%(...) as float
 *     dim var%(...) as string
 *
 *     local dim var%(...) as double integer
 *     local dim var%(...) as integer
 *     local dim var%(...) as dword
 *     local dim var%(...) as byte
 *     local dim var%(...) as boolean
 *     local dim var%(...) as double float
 *     local dim var%(...) as float
 *     local dim var%(...) as string
 *
 *     global dim var%(...) as double integer
 *     global dim var%(...) as integer
 *     global dim var%(...) as dword
 *     global dim var%(...) as byte
 *     global dim var%(...) as boolean
 *     global dim var%(...) as double float
 *     global dim var%(...) as float
 *     global dim var%(...) as string
 */
INVALID_AS_TYPE_ALL_SCOPES(percent, double_integer);
INVALID_AS_TYPE_ALL_SCOPES(percent, integer);
INVALID_AS_TYPE_ALL_SCOPES(percent, dword);
INVALID_AS_TYPE_ALL_SCOPES(percent, byte);
INVALID_AS_TYPE_ALL_SCOPES(percent, boolean);
INVALID_AS_TYPE_ALL_SCOPES(percent, double_float);
INVALID_AS_TYPE_ALL_SCOPES(percent, float);
INVALID_AS_TYPE_ALL_SCOPES(percent, string);
/*
 *     dim var!(...) as double integer
 *     dim var!(...) as integer
 *     dim var!(...) as dword
 *     dim var!(...) as word
 *     dim var!(...) as byte
 *     dim var!(...) as boolean
 *     dim var!(...) as float
 *     dim var!(...) as string
 *
 *     local dim var!(...) as double integer
 *     local dim var!(...) as integer
 *     local dim var!(...) as dword
 *     local dim var!(...) as word
 *     local dim var!(...) as byte
 *     local dim var!(...) as boolean
 *     local dim var!(...) as float
 *     local dim var!(...) as string
 *
 *     global dim var!(...) as double integer
 *     global dim var!(...) as integer
 *     global dim var!(...) as dword
 *     global dim var!(...) as word
 *     global dim var!(...) as byte
 *     global dim var!(...) as boolean
 *     global dim var!(...) as float
 *     global dim var!(...) as string
 */
INVALID_AS_TYPE_ALL_SCOPES(excl, double_integer);
INVALID_AS_TYPE_ALL_SCOPES(excl, integer);
INVALID_AS_TYPE_ALL_SCOPES(excl, dword);
INVALID_AS_TYPE_ALL_SCOPES(excl, word);
INVALID_AS_TYPE_ALL_SCOPES(excl, byte);
INVALID_AS_TYPE_ALL_SCOPES(excl, boolean);
INVALID_AS_TYPE_ALL_SCOPES(excl, float);
INVALID_AS_TYPE_ALL_SCOPES(excl, string);
/*
 *     dim var#(...) as double integer
 *     dim var#(...) as integer
 *     dim var#(...) as dword
 *     dim var#(...) as word
 *     dim var#(...) as byte
 *     dim var#(...) as boolean
 *     dim var#(...) as double float
 *     dim var#(...) as string
 *
 *     local dim var#(...) as double integer
 *     local dim var#(...) as integer
 *     local dim var#(...) as dword
 *     local dim var#(...) as word
 *     local dim var#(...) as byte
 *     local dim var#(...) as boolean
 *     local dim var#(...) as double float
 *     local dim var#(...) as string
 *
 *     global dim var#(...) as double integer
 *     global dim var#(...) as integer
 *     global dim var#(...) as dword
 *     global dim var#(...) as word
 *     global dim var#(...) as byte
 *     global dim var#(...) as boolean
 *     global dim var#(...) as double float
 *     global dim var#(...) as string
 */
INVALID_AS_TYPE_ALL_SCOPES(hash, double_integer);
INVALID_AS_TYPE_ALL_SCOPES(hash, integer);
INVALID_AS_TYPE_ALL_SCOPES(hash, dword);
INVALID_AS_TYPE_ALL_SCOPES(hash, word);
INVALID_AS_TYPE_ALL_SCOPES(hash, byte);
INVALID_AS_TYPE_ALL_SCOPES(hash, boolean);
INVALID_AS_TYPE_ALL_SCOPES(hash, double_float);
INVALID_AS_TYPE_ALL_SCOPES(hash, string);
/*
 *     dim var$(...) as double integer
 *     dim var$(...) as integer
 *     dim var$(...) as dword
 *     dim var$(...) as word
 *     dim var$(...) as byte
 *     dim var$(...) as boolean
 *     dim var$(...) as double float
 *     dim var$(...) as float
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
 *     global dim var$(...) as double integer
 *     global dim var$(...) as integer
 *     global dim var$(...) as dword
 *     global dim var$(...) as word
 *     global dim var$(...) as byte
 *     global dim var$(...) as boolean
 *     global dim var$(...) as double float
 *     global dim var$(...) as float
 */
INVALID_AS_TYPE_ALL_SCOPES(dollar, double_integer);
INVALID_AS_TYPE_ALL_SCOPES(dollar, integer);
INVALID_AS_TYPE_ALL_SCOPES(dollar, dword);
INVALID_AS_TYPE_ALL_SCOPES(dollar, word);
INVALID_AS_TYPE_ALL_SCOPES(dollar, byte);
INVALID_AS_TYPE_ALL_SCOPES(dollar, boolean);
INVALID_AS_TYPE_ALL_SCOPES(dollar, double_float);
INVALID_AS_TYPE_ALL_SCOPES(dollar, float);
/*
 *     dim var()
 *     dim var&()
 *     dim var%()
 *     dim var!()
 *     dim var#()
 *     dim var$()
 *     local dim var()
 *     local dim var&()
 *     local dim var%()
 *     local dim var!()
 *     local dim var#()
 *     local dim var$()
 *     global dim var()
 *     global dim var&()
 *     global dim var%()
 *     global dim var!()
 *     global dim var#()
 *     global dim var$()
 */
#define INVALID_ZERO_SIZE_ARRAY(scope, ann)                                   \
TEST_F(NAME, scope##_dim_arr_##ann##_without_arguments_is_invalid)            \
{                                                                             \
    ast = driver->parse("test", scope##_str " dim arr" ann##_str "()", matcher);\
    ASSERT_THAT(ast, IsNull());                                               \
}
#define INVALID_ZERO_SIZE_ARRAY_ALL_SCOPES(ann)                               \
    INVALID_ZERO_SIZE_ARRAY(none, ann)                                        \
    INVALID_ZERO_SIZE_ARRAY(local, ann)                                       \
    INVALID_ZERO_SIZE_ARRAY(global, ann)
INVALID_ZERO_SIZE_ARRAY_ALL_SCOPES(none)
INVALID_ZERO_SIZE_ARRAY_ALL_SCOPES(amp)
INVALID_ZERO_SIZE_ARRAY_ALL_SCOPES(percent)
INVALID_ZERO_SIZE_ARRAY_ALL_SCOPES(excl)
INVALID_ZERO_SIZE_ARRAY_ALL_SCOPES(hash)
INVALID_ZERO_SIZE_ARRAY_ALL_SCOPES(dollar)
/*
 *     dim var() as double integer
 *     dim var() as integer
 *     dim var() as dword
 *     dim var() as word
 *     dim var() as byte
 *     dim var() as boolean
 *     dim var() as double float
 *     dim var() as float
 *     dim var() as string
 *     dim var&() as double integer
 *     dim var%() as word
 *     dim var!() as double float
 *     dim var#() as float
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
 *     local dim var() as string
 *     local dim var&() as double integer
 *     local dim var%() as word
 *     local dim var!() as double float
 *     local dim var#() as float
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
 *     global dim var() as string
 *     global dim var&() as double integer
 *     global dim var%() as word
 *     global dim var!() as double float
 *     global dim var#() as float
 *     global dim var$() as string
 */
#define INVALID_ZERO_SIZE_ARRAY_AS_TYPE(scope, ann, type)                     \
TEST_F(NAME, scope##_dim_arr_##ann##_as_##type##_without_arguments_is_invalid)\
{                                                                             \
    ast = driver->parse("test", scope##_str " dim arr" ann##_str "() as " type##_str, matcher);\
    ASSERT_THAT(ast, IsNull());                                               \
}
#define INVALID_ZERO_SIZE_ARRAY_AS_TYPE_ALL_SCOPES(ann, type)                 \
    INVALID_ZERO_SIZE_ARRAY_AS_TYPE(none, ann, type)                          \
    INVALID_ZERO_SIZE_ARRAY_AS_TYPE(local, ann, type)                         \
    INVALID_ZERO_SIZE_ARRAY_AS_TYPE(global, ann, type)
INVALID_ZERO_SIZE_ARRAY_AS_TYPE_ALL_SCOPES(none, double_integer)
INVALID_ZERO_SIZE_ARRAY_AS_TYPE_ALL_SCOPES(none, integer)
INVALID_ZERO_SIZE_ARRAY_AS_TYPE_ALL_SCOPES(none, dword)
INVALID_ZERO_SIZE_ARRAY_AS_TYPE_ALL_SCOPES(none, word)
INVALID_ZERO_SIZE_ARRAY_AS_TYPE_ALL_SCOPES(none, byte)
INVALID_ZERO_SIZE_ARRAY_AS_TYPE_ALL_SCOPES(none, boolean)
INVALID_ZERO_SIZE_ARRAY_AS_TYPE_ALL_SCOPES(none, double_float)
INVALID_ZERO_SIZE_ARRAY_AS_TYPE_ALL_SCOPES(none, float)
INVALID_ZERO_SIZE_ARRAY_AS_TYPE_ALL_SCOPES(none, string)
INVALID_ZERO_SIZE_ARRAY_AS_TYPE_ALL_SCOPES(amp, double_integer)
INVALID_ZERO_SIZE_ARRAY_AS_TYPE_ALL_SCOPES(percent, word)
INVALID_ZERO_SIZE_ARRAY_AS_TYPE_ALL_SCOPES(excl, double_float)
INVALID_ZERO_SIZE_ARRAY_AS_TYPE_ALL_SCOPES(hash, float)
INVALID_ZERO_SIZE_ARRAY_AS_TYPE_ALL_SCOPES(dollar, string)
