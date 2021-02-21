#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/commands/Command.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"

#define NAME db_parser_var_decl

using namespace testing;
using namespace odb;

class NAME : public ParserTestHarness
{
public:
};

// Scope specifiers
#define global_str "global"
#define local_str "local"
#define none_str ""

// Scope enums
#define global_scope ast::Symbol::Scope::GLOBAL
#define local_scope ast::Symbol::Scope::LOCAL
#define none_scope ast::Symbol::Scope::LOCAL

// Anotations
#define amp_str "&"
#define percent_str "%"
#define excl_str "!"
#define hash_str "#"
#define dollar_str "$"

// Annotation enums
#define amp_ann ast::Symbol::Annotation::DOUBLE_INTEGER
#define percent_ann ast::Symbol::Annotation::WORD
#define hash_ann ast::Symbol::Annotation::FLOAT
#define excl_ann ast::Symbol::Annotation::DOUBLE_FLOAT
#define dollar_ann ast::Symbol::Annotation::STRING
#define none_ann ast::Symbol::Annotation::NONE

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
#define complex_str "complex"
#define mat2x2_str "mat2x2"
#define mat2x3_str "mat2x3"
#define mat2x4_str "mat2x4"
#define mat3x2_str "mat3x2"
#define mat3x3_str "mat3x3"
#define mat3x4_str "mat3x4"
#define mat4x2_str "mat4x2"
#define mat4x3_str "mat4x3"
#define mat4x4_str "mat4x4"
#define quat_str "quat"
#define vec2_str "vec2"
#define vec3_str "vec3"
#define vec4_str "vec4"

// Type initial values
#define double_integer_initial_value   0
#define integer_initial_value          0
#define dword_initial_value            0
#define word_initial_value             0
#define byte_initial_value             0
#define boolean_initial_value          false
#define double_float_initial_value     0.0
#define float_initial_value            0.0f
#define string_initial_value           ""
#define complex_initial_value          {0, 0}
#define mat2x2_initial_value           {{1, 0}, {0, 1}}
#define mat2x3_initial_value           {{0, 0}, {0, 0}, {0, 0}}
#define mat2x4_initial_value           {{0, 0}, {0, 0}, {0, 0}, {0, 0}}
#define mat3x2_initial_value           {{0, 0, 0}, {0, 0, 0}}
#define mat3x3_initial_value           {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}
#define mat3x4_initial_value           {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}
#define mat4x2_initial_value           {{0, 0, 0, 0}, {0, 0, 0, 0}}
#define mat4x3_initial_value           {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}
#define mat4x4_initial_value           {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}
#define quat_initial_value             {0, 0, 0, 1}
#define vec2_initial_value             {0, 0}
#define vec3_initial_value             {0, 0, 0}
#define vec4_initial_value             {0, 0, 0, 0}

// Type decl visitors
#define double_integer_decl_visitor visitDoubleIntegerVarDecl
#define integer_decl_visitor visitIntegerVarDecl
#define dword_decl_visitor visitDwordVarDecl
#define word_decl_visitor visitWordVarDecl
#define byte_decl_visitor visitByteVarDecl
#define boolean_decl_visitor visitBooleanVarDecl
#define double_float_decl_visitor visitDoubleFloatVarDecl
#define float_decl_visitor visitFloatVarDecl
#define string_decl_visitor visitStringVarDecl

// type literal visitor
#define double_integer_literal_visitor visitDoubleIntegerLiteral
#define integer_literal_visitor visitIntegerLiteral
#define dword_literal_visitor visitDwordLiteral
#define word_literal_visitor visitWordLiteral
#define byte_literal_visitor visitByteLiteral
#define boolean_literal_visitor visitBooleanLiteral
#define double_float_literal_visitor visitDoubleFloatLiteral
#define float_literal_visitor visitFloatLiteral
#define string_literal_visitor visitStringLiteral

// type literal comparisons
#define double_integer_literal_eq DoubleIntegerLiteralEq
#define integer_literal_eq IntegerLiteralEq
#define dword_literal_eq DwordLiteralEq
#define word_literal_eq WordLiteralEq
#define byte_literal_eq ByteLiteralEq
#define boolean_literal_eq BooleanLiteralEq
#define double_float_literal_eq DoubleFloatLiteralEq
#define float_literal_eq FloatLiteralEq
#define string_literal_eq StringLiteralEq

/*
 * All possible valid variable declarations:
 *
 *     local var
 *     local var&
 *     local var%
 *     local var!
 *     local var#
 *     local var$
 *     global var
 *     global var&
 *     global var%
 *     global var!
 *     global var#
 *     global var$
 */
#define VALID(scope, ann, type)                                               \
TEST_F(NAME, scope##_var_##ann##_defaults_to_##type)                          \
{                                                                             \
    ast = driver->parse("test", scope##_str " var" ann##_str, matcher);       \
    ASSERT_THAT(ast, NotNull());                                              \
                                                                              \
    StrictMock<ASTMockVisitor> v;                                             \
    Expectation exp;                                                          \
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));                   \
    exp = EXPECT_CALL(v, type##_decl_visitor(_)).After(exp);                  \
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(                          \
        ScopedAnnotatedSymbolEq(scope##_scope, ann##_ann, "var"))).After(exp);\
    exp = EXPECT_CALL(v, type##_literal_visitor(type##_literal_eq(type##_initial_value))).After(exp);\
                                                                              \
    ast->accept(&v);                                                          \
}
VALID(local, none, integer)
VALID(local, amp, double_integer)
VALID(local, percent, word)
VALID(local, excl, double_float)
VALID(local, hash, float)
VALID(local, dollar, string)
VALID(global, none, integer)
VALID(global, amp, double_integer)
VALID(global, percent, word)
VALID(global, excl, double_float)
VALID(global, hash, float)
VALID(global, dollar, string)
/*
 *     local var = x
 *     local var& = x
 *     local var% = x
 *     local var# = x
 *     local var! = x
 *     local var$ = x
 *     global var = x
 *     global var& = x
 *     global var% = x
 *     global var! = x
 *     global var# = x
 *     global var$ = x
 */
#define VALID_INITIAL(scope, ann, type)                                       \
TEST_F(NAME, scope##_var_##ann##_with_assignment_defaults_to_##type)          \
{                                                                             \
    ast = driver->parse("test", scope##_str " var" ann##_str " = 5.4", matcher);\
    ASSERT_THAT(ast, NotNull());                                              \
                                                                              \
    StrictMock<ASTMockVisitor> v;                                             \
    Expectation exp;                                                          \
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));                   \
    exp = EXPECT_CALL(v, type##_decl_visitor(_)).After(exp);                  \
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(                          \
        ScopedAnnotatedSymbolEq(scope##_scope, ann##_ann, "var"))).After(exp);\
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp);\
                                                                              \
    ast->accept(&v);                                                          \
}
VALID_INITIAL(local, none, integer)
VALID_INITIAL(local, amp, double_integer)
VALID_INITIAL(local, percent, word)
VALID_INITIAL(local, excl, double_float)
VALID_INITIAL(local, hash, float)
VALID_INITIAL(local, dollar, string)
VALID_INITIAL(global, none, integer)
VALID_INITIAL(global, amp, double_integer)
VALID_INITIAL(global, percent, word)
VALID_INITIAL(global, excl, double_float)
VALID_INITIAL(global, hash, float)
VALID_INITIAL(global, dollar, string)
/*
 *     var as double integer
 *     var as integer
 *     var as dword
 *     var as word
 *     var as byte
 *     var as boolean
 *     var as double float
 *     var as float
 *     var as string
 *     var& as double integer
 *     var% as word
 *     var! as double float
 *     var# as float
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
 *     local var as string
 *     local var& as double integer
 *     local var% as word
 *     local var! as double float
 *     local var# as float
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
 *     global var as string
 *     global var& as double integer
 *     global var% as word
 *     global var! as double float
 *     global var# as float
 *     global var$ as string
 */
#define VALID_AS_TYPE(scope, ann, as_type)                                    \
TEST_F(NAME, scope##_var_##ann##_as_##as_type)                                \
{                                                                             \
    ast = driver->parse("test", scope##_str " var" ann##_str " as " as_type##_str, matcher);\
    ASSERT_THAT(ast, NotNull());                                              \
                                                                              \
    StrictMock<ASTMockVisitor> v;                                             \
    Expectation exp;                                                          \
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));                   \
    exp = EXPECT_CALL(v, as_type##_decl_visitor(_)).After(exp);               \
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(                          \
        ScopedAnnotatedSymbolEq(scope##_scope, ann##_ann, "var"))).After(exp);\
    exp = EXPECT_CALL(v, as_type##_literal_visitor(as_type##_literal_eq(as_type##_initial_value))).After(exp);\
                                                                              \
    ast->accept(&v);                                                          \
}
#define VALID_AS_TYPE_ALL_SCOPES(ann, as_type)                                \
    VALID_AS_TYPE(none, ann, as_type)                                         \
    VALID_AS_TYPE(global, ann, as_type)                                       \
    VALID_AS_TYPE(local, ann, as_type)
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
 *     var as double integer = x
 *     var as integer = x
 *     var as dword = x
 *     var as word = x
 *     var as byte = x
 *     var as boolean = x
 *     var as double float = x
 *     var as float = x
 *     var as string = x
 *     var& as double integer = x
 *     var% as word = x
 *     var! as double float = x
 *     var# as float = x
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
 *     local var as string = x
 *     local var& as double integer = x
 *     local var% as word = x
 *     local var! as double float = x
 *     local var# as float = x
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
 *     global var as string = x
 *     global var& as double integer = x
 *     global var% as word = x
 *     global var! as double float = x
 *     global var# as float = x
 *     global var$ as string = x
 */
#define VALID_AS_TYPE_INITIAL_ALL_SCOPES(ann, as_type)                        \
    VALID_AS_TYPE_INITIAL(none, ann, as_type)                                 \
    VALID_AS_TYPE_INITIAL(global, ann, as_type)                               \
    VALID_AS_TYPE_INITIAL(local, ann, as_type)
#define VALID_AS_TYPE_INITIAL(scope, ann, as_type)                            \
TEST_F(NAME, scope##_var_##ann##_as_##as_type##_with_initial_value)           \
{                                                                             \
    ast = driver->parse("test", scope##_str " var" ann##_str " as " as_type##_str " = 5.4", matcher); \
    ASSERT_THAT(ast, NotNull());                                              \
                                                                              \
    StrictMock<ASTMockVisitor> v;                                             \
    Expectation exp;                                                          \
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));                   \
    exp = EXPECT_CALL(v, as_type##_decl_visitor(_)).After(exp);               \
    exp = EXPECT_CALL(v, visitScopedAnnotatedSymbol(                          \
        ScopedAnnotatedSymbolEq(scope##_scope, ann##_ann, "var"))).After(exp);\
    exp = EXPECT_CALL(v, visitDoubleFloatLiteral(DoubleFloatLiteralEq(5.4))).After(exp); \
                                                                              \
    ast->accept(&v);                                                          \
}
VALID_AS_TYPE_INITIAL_ALL_SCOPES(none, double_integer)
VALID_AS_TYPE_INITIAL_ALL_SCOPES(none, integer)
VALID_AS_TYPE_INITIAL_ALL_SCOPES(none, dword)
VALID_AS_TYPE_INITIAL_ALL_SCOPES(none, word)
VALID_AS_TYPE_INITIAL_ALL_SCOPES(none, byte)
VALID_AS_TYPE_INITIAL_ALL_SCOPES(none, boolean)
VALID_AS_TYPE_INITIAL_ALL_SCOPES(none, double_float)
VALID_AS_TYPE_INITIAL_ALL_SCOPES(none, float)
VALID_AS_TYPE_INITIAL_ALL_SCOPES(none, string)
VALID_AS_TYPE_INITIAL_ALL_SCOPES(amp, double_integer)
VALID_AS_TYPE_INITIAL_ALL_SCOPES(percent, word)
VALID_AS_TYPE_INITIAL_ALL_SCOPES(excl, double_float)
VALID_AS_TYPE_INITIAL_ALL_SCOPES(hash, float)
VALID_AS_TYPE_INITIAL_ALL_SCOPES(dollar, string)

/*
 * Invalid variable declarations:
 *
 *     var
 *     var&
 *     var%
 *     var#
 *     var!
 *     var$
 */
#define INVALID(ann)                                                          \
TEST_F(NAME, var_##ann##_alone_is_not_a_valid_statement)                      \
{                                                                             \
    ast = driver->parse("test", "var" ann##_str, matcher);                    \
    ASSERT_THAT(ast, IsNull());                                               \
}
INVALID(none)
INVALID(amp)
INVALID(percent)
INVALID(hash)
INVALID(excl)
INVALID(dollar)
/*
 *     var& as integer
 *     var& as dword
 *     var& as word
 *     var& as byte
 *     var& as boolean
 *     var& as double float
 *     var& as float
 *     var& as string
 *
 *     local var& as integer
 *     local var& as dword
 *     local var& as word
 *     local var& as byte
 *     local var& as boolean
 *     local var& as double float
 *     local var& as float
 *     local var& as string
 *
 *     global var& as integer
 *     global var& as dword
 *     global var& as word
 *     global var& as byte
 *     global var& as boolean
 *     global var& as double float
 *     global var& as float
 *     global var& as string
 */
#define INVALID_AS_TYPE(scope, ann, as_type)                                  \
TEST_F(NAME, scope##_var_##ann##_as_##as_type##_is_invalid)                   \
{                                                                             \
    ast = driver->parse("test", scope##_str " var" ann##_str " as " as_type##_str, matcher);\
    ASSERT_THAT(ast, IsNull());                                               \
}
#define INVALID_AS_TYPE_ALL_SCOPES(ann, as_type)                              \
    INVALID_AS_TYPE(none, ann, as_type)                                       \
    INVALID_AS_TYPE(global, ann, as_type)                                     \
    INVALID_AS_TYPE(local, ann, as_type)
INVALID_AS_TYPE_ALL_SCOPES(amp, integer)
INVALID_AS_TYPE_ALL_SCOPES(amp, dword)
INVALID_AS_TYPE_ALL_SCOPES(amp, word)
INVALID_AS_TYPE_ALL_SCOPES(amp, byte)
INVALID_AS_TYPE_ALL_SCOPES(amp, boolean)
INVALID_AS_TYPE_ALL_SCOPES(amp, double_float)
INVALID_AS_TYPE_ALL_SCOPES(amp, float)
INVALID_AS_TYPE_ALL_SCOPES(amp, string)
/*
 *     var% as double integer
 *     var% as integer
 *     var% as dword
 *     var% as byte
 *     var% as boolean
 *     var% as double float
 *     var% as float
 *     var% as string
 *
 *     local var% as double integer
 *     local var% as integer
 *     local var% as dword
 *     local var% as byte
 *     local var% as boolean
 *     local var% as double float
 *     local var% as float
 *     local var% as string
 *
 *     global var% as double integer
 *     global var% as integer
 *     global var% as dword
 *     global var% as byte
 *     global var% as boolean
 *     global var% as double float
 *     global var% as float
 *     global var% as string
 */
INVALID_AS_TYPE_ALL_SCOPES(percent, double_integer)
INVALID_AS_TYPE_ALL_SCOPES(percent, integer)
INVALID_AS_TYPE_ALL_SCOPES(percent, dword)
INVALID_AS_TYPE_ALL_SCOPES(percent, byte)
INVALID_AS_TYPE_ALL_SCOPES(percent, boolean)
INVALID_AS_TYPE_ALL_SCOPES(percent, double_float)
INVALID_AS_TYPE_ALL_SCOPES(percent, float)
INVALID_AS_TYPE_ALL_SCOPES(percent, string)
/*
 *     var# as double integer
 *     var# as integer
 *     var# as dword
 *     var# as word
 *     var# as byte
 *     var# as boolean
 *     var# as double float
 *     var# as string
 *
 *     local var# as double integer
 *     local var# as integer
 *     local var# as dword
 *     local var# as word
 *     local var# as byte
 *     local var# as boolean
 *     local var# as double float
 *     local var# as string
 *
 *     global var# as double integer
 *     global var# as integer
 *     global var# as dword
 *     global var# as word
 *     global var# as byte
 *     global var# as boolean
 *     global var# as double float
 *     global var# as string
 */
INVALID_AS_TYPE_ALL_SCOPES(hash, double_integer)
INVALID_AS_TYPE_ALL_SCOPES(hash, integer)
INVALID_AS_TYPE_ALL_SCOPES(hash, dword)
INVALID_AS_TYPE_ALL_SCOPES(hash, word)
INVALID_AS_TYPE_ALL_SCOPES(hash, byte)
INVALID_AS_TYPE_ALL_SCOPES(hash, boolean)
INVALID_AS_TYPE_ALL_SCOPES(hash, double_float)
INVALID_AS_TYPE_ALL_SCOPES(hash, string)
/*
 *     var! as double integer
 *     var! as integer
 *     var! as dword
 *     var! as word
 *     var! as byte
 *     var! as boolean
 *     var! as float
 *     var! as string
 *
 *     local var! as double integer
 *     local var! as integer
 *     local var! as dword
 *     local var! as word
 *     local var! as byte
 *     local var! as boolean
 *     local var! as float
 *     local var! as string
 *
 *     global var! as double integer
 *     global var! as integer
 *     global var! as dword
 *     global var! as word
 *     global var! as byte
 *     global var! as boolean
 *     global var! as float
 *     global var! as string
 */
INVALID_AS_TYPE_ALL_SCOPES(excl, double_integer)
INVALID_AS_TYPE_ALL_SCOPES(excl, integer)
INVALID_AS_TYPE_ALL_SCOPES(excl, dword)
INVALID_AS_TYPE_ALL_SCOPES(excl, word)
INVALID_AS_TYPE_ALL_SCOPES(excl, byte)
INVALID_AS_TYPE_ALL_SCOPES(excl, boolean)
INVALID_AS_TYPE_ALL_SCOPES(excl, float)
INVALID_AS_TYPE_ALL_SCOPES(excl, string)
/*
 *     var$ as double integer
 *     var$ as integer
 *     var$ as dword
 *     var$ as word
 *     var$ as byte
 *     var$ as boolean
 *     var$ as double float
 *     var$ as float
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
 *     global var$ as double integer
 *     global var$ as integer
 *     global var$ as dword
 *     global var$ as word
 *     global var$ as byte
 *     global var$ as boolean
 *     global var$ as double float
 *     global var$ as float
 */
INVALID_AS_TYPE_ALL_SCOPES(dollar, double_integer)
INVALID_AS_TYPE_ALL_SCOPES(dollar, integer)
INVALID_AS_TYPE_ALL_SCOPES(dollar, dword)
INVALID_AS_TYPE_ALL_SCOPES(dollar, word)
INVALID_AS_TYPE_ALL_SCOPES(dollar, byte)
INVALID_AS_TYPE_ALL_SCOPES(dollar, boolean)
INVALID_AS_TYPE_ALL_SCOPES(dollar, double_float)
INVALID_AS_TYPE_ALL_SCOPES(dollar, float)
