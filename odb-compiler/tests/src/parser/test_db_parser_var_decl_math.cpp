#include "odb-compiler/ast/Annotation.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/Scope.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/matchers/BinaryOpEq.hpp"
#include "odb-compiler/tests/matchers/BlockStmntCountEq.hpp"
#include "odb-compiler/tests/matchers/InitializerListCountEq.hpp"
#include "odb-compiler/tests/matchers/LiteralEq.hpp"
#include "odb-compiler/tests/matchers/ScopedIdentifierEq.hpp"
#include "odb-compiler/tests/matchers/VarDeclEq.hpp"

#define NAME db_parser_var_decl

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
#define quat_initial_value             {1, 0, 0, 0}
#define vec2_initial_value             {0, 0}
#define vec3_initial_value             {0, 0, 0}
#define vec4_initial_value             {0, 0, 0, 0}

// Type names
#define complex_type Complex
#define mat2x2_type Mat2x2
#define mat2x3_type Mat2x3
#define mat2x4_type Mat2x4
#define mat3x2_type Mat3x2
#define mat3x3_type Mat3x3
#define mat3x4_type Mat3x4
#define mat4x2_type Mat4x2
#define mat4x3_type Mat4x3
#define mat4x4_type Mat4x4
#define quat_type Quat
#define vec2_type Vec2
#define vec3_type Vec3
#define vec4_type Vec4

// type literal visitor
#define complex_literal_visitor visitComplexLiteral
#define mat2x2_literal_visitor visitMat2x2Literal
#define mat2x3_literal_visitor visitMat2x3Literal
#define mat2x4_literal_visitor visitMat2x4Literal
#define mat3x2_literal_visitor visitMat3x2Literal
#define mat3x3_literal_visitor visitMat3x3Literal
#define mat3x4_literal_visitor visitMat3x4Literal
#define mat4x2_literal_visitor visitMat4x2Literal
#define mat4x3_literal_visitor visitMat4x3Literal
#define mat4x4_literal_visitor visitMat4x4Literal
#define quat_literal_visitor visitQuatLiteral
#define vec2_literal_visitor visitVec2Literal
#define vec3_literal_visitor visitVec3Literal
#define vec4_literal_visitor visitVec4Literal

// type literal comparisons
#define complex_literal_eq ComplexLiteralEq
#define mat2x2_literal_eq Mat2x2LiteralEq
#define mat2x3_literal_eq Mat2x3LiteralEq
#define mat2x4_literal_eq Mat2x4LiteralEq
#define mat3x2_literal_eq Mat3x2LiteralEq
#define mat3x3_literal_eq Mat3x3LiteralEq
#define mat3x4_literal_eq Mat3x4LiteralEq
#define mat4x2_literal_eq Mat4x2LiteralEq
#define mat4x3_literal_eq Mat4x3LiteralEq
#define mat4x4_literal_eq Mat4x4LiteralEq
#define quat_literal_eq QuatLiteralEq
#define vec2_literal_eq Vec2LiteralEq
#define vec3_literal_eq Vec3LiteralEq
#define vec4_literal_eq Vec4LiteralEq

/*
 *     var as complex
 *     var as mat2x2
 *     var as mat2x3
 *     var as mat2x4
 *     var as mat3x2
 *     var as mat3x3
 *     var as mat3x4
 *     var as mat4x2
 *     var as mat4x3
 *     var as mat4x4
 *     var as quat
 *     var as vec2
 *     var as vec3
 *     var as vec4
 *
 *     local var as complex
 *     local var as mat2x2
 *     local var as mat2x3
 *     local var as mat2x4
 *     local var as mat3x2
 *     local var as mat3x3
 *     local var as mat3x4
 *     local var as mat4x2
 *     local var as mat4x3
 *     local var as mat4x4
 *     local var as quat
 *     local var as vec2
 *     local var as vec3
 *     local var as vec4
 *
 *     global var as complex
 *     global var as mat2x2
 *     global var as mat2x3
 *     global var as mat2x4
 *     global var as mat3x2
 *     global var as mat3x3
 *     global var as mat3x4
 *     global var as mat4x2
 *     global var as mat4x3
 *     global var as mat4x4
 *     global var as quat
 *     global var as vec2
 *     global var as vec3
 *     global var as vec4
 */
#define VALID_AS_TYPE(scope, ann, as_type)                                    \
TEST_F(NAME, scope##_var_##ann##_as_##as_type)                                \
{                                                                             \
    ast = driver->parse("test", scope##_str " var" ann##_str " as " as_type##_str, matcher);\
    ASSERT_THAT(ast, NotNull());                                              \
                                                                              \
    StrictMock<ASTMockVisitor> v;                                             \
    Expectation exp;                                                          \
    exp = EXPECT_CALL(v, visitProgram(_));                                    \
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1))).After(exp);        \
    exp = EXPECT_CALL(v, visitVarDecl(VarDeclEq(BuiltinType::as_type##_type))).After(exp);               \
    exp = EXPECT_CALL(v, visitScopedIdentifier(                          \
        ScopedIdentifierEq(scope##_scope, "var", ann##_ann))).After(exp);\
    exp = EXPECT_CALL(v, visitInitializerList(InitializerListCountEq(1))).After(exp);\
    exp = EXPECT_CALL(v, as_type##_literal_visitor(as_type##_literal_eq(as_type##_initial_value))).After(exp);\
                                                                              \
    visitAST(ast, v);                                                         \
}
#define VALID_AS_TYPE_ALL_SCOPES(ann, as_type)                                \
    VALID_AS_TYPE(none, ann, as_type)                                         \
    VALID_AS_TYPE(global, ann, as_type)                                       \
    VALID_AS_TYPE(local, ann, as_type)
VALID_AS_TYPE_ALL_SCOPES(none, complex)
VALID_AS_TYPE_ALL_SCOPES(none, mat2x2)
VALID_AS_TYPE_ALL_SCOPES(none, mat2x3)
VALID_AS_TYPE_ALL_SCOPES(none, mat2x4)
VALID_AS_TYPE_ALL_SCOPES(none, mat3x2)
VALID_AS_TYPE_ALL_SCOPES(none, mat3x3)
VALID_AS_TYPE_ALL_SCOPES(none, mat3x4)
VALID_AS_TYPE_ALL_SCOPES(none, mat4x2)
VALID_AS_TYPE_ALL_SCOPES(none, mat4x3)
VALID_AS_TYPE_ALL_SCOPES(none, mat4x4)
VALID_AS_TYPE_ALL_SCOPES(none, quat)
VALID_AS_TYPE_ALL_SCOPES(none, vec2)
VALID_AS_TYPE_ALL_SCOPES(none, vec3)
VALID_AS_TYPE_ALL_SCOPES(none, vec4)

/*
 *     var as complex = 1+2i
 *     var as complex = r, i
 *     var as mat2x2 = m00, m01,
 *                     m10, m11
 *     var as mat2x3 = m00, m01, m02,
 *                     m10, m11, m12
 *     var as mat2x4 = m00, m01, m02, m03,
 *                     m10, m11, m12, m13
 *     var as mat3x2 = m00, m01,
 *                     m10, m11,
 *                     m20, m21
 *     var as mat3x3 = m00, m01, m02,
 *                     m10, m11, m12,
 *                     m20, m21, m22
 *     var as mat3x4 = m00, m01, m02, m03,
 *                     m10, m11, m12, m13,
 *                     m20, m21, m22, m23
 *     var as mat4x2 = m00, m01,
 *                     m10, m11,
 *                     m20, m21,
 *                     m30, m31
 *     var as mat4x3 = m00, m01, m02,
 *                     m10, m11, m12,
 *                     m20, m21, m22,
 *                     m30, m31, m32
 *     var as mat4x4 = m00, m01, m02, m03,
 *                     m10, m11, m12, m13,
 *                     m20, m21, m22, m23,
 *                     m30, m31, m32, m33
 *     var as quat = 1+2i+2j+2k
 *     var as quat = w, x, y, z
 *     var as vec2 = x, y
 *     var as vec3 = x, y, z
 *     var as vec4 = x, y, z, w
 */
TEST_F(NAME, var_as_complex_with_complex_literal_initializer)
{
    using Scope = Scope;
    using Ann = Annotation;

    ast = driver->parse("test", "var as complex = 1 + 2i", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarDecl(VarDeclEq(BuiltinType::Complex))).After(exp);
    exp = EXPECT_CALL(v, visitScopedIdentifier(ScopedIdentifierEq(Scope::LOCAL, "var", Ann::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitInitializerList(InitializerListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::ADD))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitComplexLiteral(ComplexLiteralEq({0, 2}))).After(exp);

    visitAST(ast, v);
}
TEST_F(NAME, var_as_complex_with_complex_initializer_list)
{
    using Scope = Scope;
    using Ann = Annotation;

    ast = driver->parse("test", "var as complex = 1, 2", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarDecl(VarDeclEq(BuiltinType::Complex))).After(exp);
    exp = EXPECT_CALL(v, visitScopedIdentifier(ScopedIdentifierEq(Scope::LOCAL, "var", Ann::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitInitializerList(InitializerListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);

    visitAST(ast, v);
}
TEST_F(NAME, var_as_quat_with_quat_literal_initializer)
{
    using Scope = Scope;
    using Ann = Annotation;

    ast = driver->parse("test", "var as quat = 1 + 2i + 3j + 4k", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarDecl(VarDeclEq(BuiltinType::Quat))).After(exp);
    exp = EXPECT_CALL(v, visitScopedIdentifier(ScopedIdentifierEq(Scope::LOCAL, "var", Ann::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitInitializerList(InitializerListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::ADD))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::ADD))).After(exp);
    exp = EXPECT_CALL(v, visitBinaryOp(BinaryOpEq(BinaryOpType::ADD))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitComplexLiteral(ComplexLiteralEq({0, 2}))).After(exp);
    exp = EXPECT_CALL(v, visitQuatLiteral(QuatLiteralEq({0, 0, 3, 0}))).After(exp);
    exp = EXPECT_CALL(v, visitQuatLiteral(QuatLiteralEq({0, 0, 0, 4}))).After(exp);

    visitAST(ast, v);
}
TEST_F(NAME, var_as_quat_with_quat_initializer_list)
{
    using Scope = Scope;
    using Ann = Annotation;

    ast = driver->parse("test", "var as quat = 1, 2, 3, 4", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarDecl(VarDeclEq(BuiltinType::Quat))).After(exp);
    exp = EXPECT_CALL(v, visitScopedIdentifier(ScopedIdentifierEq(Scope::LOCAL, "var", Ann::NONE))).After(exp);
    exp = EXPECT_CALL(v, visitInitializerList(InitializerListCountEq(4))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(3))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(4))).After(exp);

    visitAST(ast, v);
}
/*
 *     local var as complex = 1+2i
 *     local var as complex = (r, i)
 *     local var as mat2x2 = (m00, m01,
 *                            m10, m11)
 *     local var as mat2x3 = (m00, m01, m02,
 *                            m10, m11, m12)
 *     local var as mat2x4 = (m00, m01, m02, m03,
 *                            m10, m11, m12, m13)
 *     local var as mat3x2 = (m00, m01,
 *                            m10, m11,
 *                            m20, m21)
 *     local var as mat3x3 = (m00, m01, m02,
 *                            m10, m11, m12,
 *                            m20, m21, m22)
 *     local var as mat3x4 = (m00, m01, m02, m03,
 *                            m10, m11, m12, m13,
 *                            m20, m21, m22, m23)
 *     local var as mat4x2 = (m00, m01,
 *                            m10, m11,
 *                            m20, m21,
 *                            m30, m31)
 *     local var as mat4x3 = (m00, m01, m02,
 *                            m10, m11, m12,
 *                            m20, m21, m22,
 *                            m30, m31, m32)
 *     local var as mat4x4 = (m00, m01, m02, m03,
 *                            m10, m11, m12, m13,
 *                            m20, m21, m22, m23,
 *                            m30, m31, m32, m33)
 *     local var as quat = 1+2i+2j+2k
 *     local var as quat = (w, x, y, z)
 *     local var as vec2 = (x, y)
 *     local var as vec3 = (x, y, z)
 *     local var as vec4 = (x, y, z, w)
 *
 *     global var as complex = 1+2i
 *     global var as complex = (r, i)
 *     global var as mat2x2 = (m00, m01,
 *                             m10, m11)
 *     global var as mat2x3 = (m00, m01, m02,
 *                             m10, m11, m12)
 *     global var as mat2x4 = (m00, m01, m02, m03,
 *                             m10, m11, m12, m13)
 *     global var as mat3x2 = (m00, m01,
 *                             m10, m11,
 *                             m20, m21)
 *     global var as mat3x3 = (m00, m01, m02,
 *                             m10, m11, m12,
 *                             m20, m21, m22)
 *     global var as mat3x4 = (m00, m01, m02, m03,
 *                             m10, m11, m12, m13,
 *                             m20, m21, m22, m23)
 *     global var as mat4x2 = (m00, m01,
 *                             m10, m11,
 *                             m20, m21,
 *                             m30, m31)
 *     global var as mat4x3 = (m00, m01, m02,
 *                             m10, m11, m12,
 *                             m20, m21, m22,
 *                             m30, m31, m32)
 *     global var as mat4x4 = (m00, m01, m02, m03,
 *                             m10, m11, m12, m13,
 *                             m20, m21, m22, m23,
 *                             m30, m31, m32, m33)
 *     global var as quat = 1+2i+2j+2k
 *     global var as quat = (w, x, y, z)
 *     global var as vec2 = (x, y)
 *     global var as vec3 = (x, y, z)
 *     global var as vec4 = (x, y, z, w)
 */

/*
 * Invalid variable declarations:
 *
 *     var
 *     var&
 *     var%
 *     var#
 *     var!
 *     var$
 *
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
 *
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
 *
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
 *
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
