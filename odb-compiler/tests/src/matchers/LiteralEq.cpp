#include "odb-compiler/tests/matchers/LiteralEq.hpp"
#include "odb-compiler/ast/Literal.hpp"

#define DoubleInteger_COMPARE   literal->value() == expectedValue_
#define DoubleInteger_VALUE     literal->value()
#define DoubleInteger_EXPECTED  expectedValue_

#define Integer_COMPARE         literal->value() == expectedValue_
#define Integer_VALUE           literal->value()
#define Integer_EXPECTED        expectedValue_

#define Dword_COMPARE           literal->value() == expectedValue_
#define Dword_VALUE             literal->value()
#define Dword_EXPECTED          expectedValue_

#define Word_COMPARE            literal->value() == expectedValue_
#define Word_VALUE              literal->value()
#define Word_EXPECTED           expectedValue_

#define Byte_COMPARE            literal->value() == expectedValue_
#define Byte_VALUE              literal->value()
#define Byte_EXPECTED           expectedValue_

#define Boolean_COMPARE         literal->value() == expectedValue_
#define Boolean_VALUE           literal->value()
#define Boolean_EXPECTED        expectedValue_

#define DoubleFloat_COMPARE     literal->value() == expectedValue_
#define DoubleFloat_VALUE       literal->value()
#define DoubleFloat_EXPECTED    expectedValue_

#define Float_COMPARE           literal->value() == expectedValue_
#define Float_VALUE             literal->value()
#define Float_EXPECTED          expectedValue_

#define String_COMPARE          literal->value() == expectedValue_
#define String_VALUE            literal->value()
#define String_EXPECTED         expectedValue_

#define Complex_COMPARE         literal->value().real == expectedValue_.real && literal->value().imag == expectedValue_.imag
#define Complex_VALUE           literal->value().real << " + " << literal->value().imag << "i"
#define Complex_EXPECTED        expectedValue_.real << " + " << expectedValue_.imag << "i"

#define Mat2x2_COMPARE          literal->value().e0.x == expectedValue_.e0.x && literal->value().e1.x == expectedValue_.e1.x \
                             && literal->value().e0.y == expectedValue_.e0.y && literal->value().e1.y == expectedValue_.e1.y
#define Mat2x2_VALUE            "\n[" << literal->value().e0.x << ", " << literal->value().e1.x << ";\n" \
                             << " "   << literal->value().e0.y << ", " << literal->value().e1.y << "]";
#define Mat2x2_EXPECTED         "\n[" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ";\n" \
                             << " "   << expectedValue_.e0.y << ", " << expectedValue_.e1.y << "]";

#define Mat2x3_COMPARE          literal->value().e0.x == expectedValue_.e0.x && literal->value().e1.x == expectedValue_.e1.x && literal->value().e2.x == expectedValue_.e2.x \
                             && literal->value().e0.y == expectedValue_.e0.y && literal->value().e1.y == expectedValue_.e1.y && literal->value().e2.y == expectedValue_.e2.y
#define Mat2x3_VALUE            "\n[" << literal->value().e0.x << ", " << literal->value().e1.x << ", " << literal->value().e2.x << ";\n" \
                             << " "   << literal->value().e0.y << ", " << literal->value().e1.y << ", " << literal->value().e2.y << "]";
#define Mat2x3_EXPECTED         "\n[" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ", " << expectedValue_.e2.x << ";\n" \
                             << " "   << expectedValue_.e0.y << ", " << expectedValue_.e1.y << ", " << expectedValue_.e2.y << "]";

#define Mat2x4_COMPARE          literal->value().e0.x == expectedValue_.e0.x && literal->value().e1.x == expectedValue_.e1.x && literal->value().e2.x == expectedValue_.e2.x && literal->value().e3.x == expectedValue_.e3.x \
                             && literal->value().e0.y == expectedValue_.e0.y && literal->value().e1.y == expectedValue_.e1.y && literal->value().e2.y == expectedValue_.e2.y && literal->value().e3.y == expectedValue_.e3.y
#define Mat2x4_VALUE            "\n[" << literal->value().e0.x << ", " << literal->value().e1.x << ", " << literal->value().e2.x << ", " << literal->value().e3.x << ";\n" \
                             << " "   << literal->value().e0.y << ", " << literal->value().e1.y << ", " << literal->value().e2.y << ", " << literal->value().e3.y << "]";
#define Mat2x4_EXPECTED         "\n[" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ", " << expectedValue_.e2.x << ", " << expectedValue_.e3.x << ";\n" \
                             << " "   << expectedValue_.e0.y << ", " << expectedValue_.e1.y << ", " << expectedValue_.e2.y << ", " << expectedValue_.e3.y << "]";

#define Mat3x2_COMPARE          literal->value().e0.x == expectedValue_.e0.x && literal->value().e1.x == expectedValue_.e1.x \
                             && literal->value().e0.y == expectedValue_.e0.y && literal->value().e1.y == expectedValue_.e1.y \
                             && literal->value().e0.z == expectedValue_.e0.z && literal->value().e1.z == expectedValue_.e1.z
#define Mat3x2_VALUE            "\n[" << literal->value().e0.x << ", " << literal->value().e1.x << ";\n" \
                             << " "   << literal->value().e0.y << ", " << literal->value().e1.y << ";\n" \
                             << " "   << literal->value().e0.z << ", " << literal->value().e1.z << "]";
#define Mat3x2_EXPECTED         "\n[" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ";\n" \
                             << " "   << expectedValue_.e0.y << ", " << expectedValue_.e1.y << ";\n" \
                             << " "   << expectedValue_.e0.z << ", " << expectedValue_.e1.z << "]";

#define Mat3x3_COMPARE          literal->value().e0.x == expectedValue_.e0.x && literal->value().e1.x == expectedValue_.e1.x && literal->value().e2.x == expectedValue_.e2.x \
                             && literal->value().e0.y == expectedValue_.e0.y && literal->value().e1.y == expectedValue_.e1.y && literal->value().e2.y == expectedValue_.e2.y \
                             && literal->value().e0.z == expectedValue_.e0.z && literal->value().e1.z == expectedValue_.e1.z && literal->value().e2.z == expectedValue_.e2.z
#define Mat3x3_VALUE            "\n[" << literal->value().e0.x << ", " << literal->value().e1.x << ", " << literal->value().e2.x << ";\n" \
                             << " "   << literal->value().e0.y << ", " << literal->value().e1.y << ", " << literal->value().e2.y << ";\n" \
                             << " "   << literal->value().e0.z << ", " << literal->value().e1.z << ", " << literal->value().e2.z << "]";
#define Mat3x3_EXPECTED         "\n[" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ", " << expectedValue_.e2.x << ";\n" \
                             << " "   << expectedValue_.e0.y << ", " << expectedValue_.e1.y << ", " << expectedValue_.e2.y << ";\n" \
                             << " "   << expectedValue_.e0.z << ", " << expectedValue_.e1.z << ", " << expectedValue_.e2.z << "]";

#define Mat3x4_COMPARE          literal->value().e0.x == expectedValue_.e0.x && literal->value().e1.x == expectedValue_.e1.x && literal->value().e2.x == expectedValue_.e2.x && literal->value().e3.x == expectedValue_.e3.x \
                             && literal->value().e0.y == expectedValue_.e0.y && literal->value().e1.y == expectedValue_.e1.y && literal->value().e2.y == expectedValue_.e2.y && literal->value().e3.y == expectedValue_.e3.y \
                             && literal->value().e0.z == expectedValue_.e0.z && literal->value().e1.z == expectedValue_.e1.z && literal->value().e2.z == expectedValue_.e2.z && literal->value().e3.z == expectedValue_.e3.z
#define Mat3x4_VALUE            "\n[" << literal->value().e0.x << ", " << literal->value().e1.x << ", " << literal->value().e2.x << ", " << literal->value().e3.x << ";\n" \
                             << " "   << literal->value().e0.y << ", " << literal->value().e1.y << ", " << literal->value().e2.y << ", " << literal->value().e3.y << ";\n" \
                             << " "   << literal->value().e0.z << ", " << literal->value().e1.z << ", " << literal->value().e2.z << ", " << literal->value().e3.z << "]";
#define Mat3x4_EXPECTED         "\n[" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ", " << expectedValue_.e2.x << ", " << expectedValue_.e3.x << ";\n" \
                             << " "   << expectedValue_.e0.y << ", " << expectedValue_.e1.y << ", " << expectedValue_.e2.y << ", " << expectedValue_.e3.y << ";\n" \
                             << " "   << expectedValue_.e0.z << ", " << expectedValue_.e1.z << ", " << expectedValue_.e2.z << ", " << expectedValue_.e3.z << "]";

#define Mat4x2_COMPARE          literal->value().e0.x == expectedValue_.e0.x && literal->value().e1.x == expectedValue_.e1.x \
                             && literal->value().e0.y == expectedValue_.e0.y && literal->value().e1.y == expectedValue_.e1.y \
                             && literal->value().e0.z == expectedValue_.e0.z && literal->value().e1.z == expectedValue_.e1.z \
                             && literal->value().e0.w == expectedValue_.e0.w && literal->value().e1.w == expectedValue_.e1.w
#define Mat4x2_VALUE            "\n[" << literal->value().e0.x << ", " << literal->value().e1.x << ";\n" \
                             << " "   << literal->value().e0.y << ", " << literal->value().e1.y << ";\n" \
                             << " "   << literal->value().e0.z << ", " << literal->value().e1.z << ";\n" \
                             << " "   << literal->value().e0.w << ", " << literal->value().e1.w << "]";
#define Mat4x2_EXPECTED         "\n[" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ";\n" \
                             << " "   << expectedValue_.e0.y << ", " << expectedValue_.e1.y << ";\n" \
                             << " "   << expectedValue_.e0.z << ", " << expectedValue_.e1.z << ";\n" \
                             << " "   << expectedValue_.e0.w << ", " << expectedValue_.e1.w << "]";

#define Mat4x3_COMPARE          literal->value().e0.x == expectedValue_.e0.x && literal->value().e1.x == expectedValue_.e1.x && literal->value().e2.x == expectedValue_.e2.x \
                             && literal->value().e0.y == expectedValue_.e0.y && literal->value().e1.y == expectedValue_.e1.y && literal->value().e2.y == expectedValue_.e2.y \
                             && literal->value().e0.z == expectedValue_.e0.z && literal->value().e1.z == expectedValue_.e1.z && literal->value().e2.z == expectedValue_.e2.z \
                             && literal->value().e0.w == expectedValue_.e0.w && literal->value().e1.w == expectedValue_.e1.w && literal->value().e2.w == expectedValue_.e2.w
#define Mat4x3_VALUE            "\n[" << literal->value().e0.x << ", " << literal->value().e1.x << ", " << literal->value().e2.x << ";\n" \
                             << " "   << literal->value().e0.y << ", " << literal->value().e1.y << ", " << literal->value().e2.y << ";\n" \
                             << " "   << literal->value().e0.z << ", " << literal->value().e1.z << ", " << literal->value().e2.z << ";\n" \
                             << " "   << literal->value().e0.w << ", " << literal->value().e1.w << ", " << literal->value().e2.w << "]";
#define Mat4x3_EXPECTED         "\n[" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ", " << expectedValue_.e2.x << ";\n" \
                             << " "   << expectedValue_.e0.y << ", " << expectedValue_.e1.y << ", " << expectedValue_.e2.y << ";\n" \
                             << " "   << expectedValue_.e0.z << ", " << expectedValue_.e1.z << ", " << expectedValue_.e2.z << ";\n" \
                             << " "   << expectedValue_.e0.w << ", " << expectedValue_.e1.w << ", " << expectedValue_.e2.w << "]";

#define Mat4x4_COMPARE          literal->value().e0.x == expectedValue_.e0.x && literal->value().e1.x == expectedValue_.e1.x && literal->value().e2.x == expectedValue_.e2.x && literal->value().e3.x == expectedValue_.e3.x \
                             && literal->value().e0.y == expectedValue_.e0.y && literal->value().e1.y == expectedValue_.e1.y && literal->value().e2.y == expectedValue_.e2.y && literal->value().e3.y == expectedValue_.e3.y \
                             && literal->value().e0.z == expectedValue_.e0.z && literal->value().e1.z == expectedValue_.e1.z && literal->value().e2.z == expectedValue_.e2.z && literal->value().e3.z == expectedValue_.e3.z \
                             && literal->value().e0.w == expectedValue_.e0.w && literal->value().e1.w == expectedValue_.e1.w && literal->value().e2.w == expectedValue_.e2.w && literal->value().e3.w == expectedValue_.e3.w
#define Mat4x4_VALUE            "\n[" << literal->value().e0.x << ", " << literal->value().e1.x << ", " << literal->value().e2.x << ", " << literal->value().e3.x << ";\n" \
                             << " "   << literal->value().e0.y << ", " << literal->value().e1.y << ", " << literal->value().e2.y << ", " << literal->value().e3.y << ";\n" \
                             << " "   << literal->value().e0.z << ", " << literal->value().e1.z << ", " << literal->value().e2.z << ", " << literal->value().e3.z << ";\n" \
                             << " "   << literal->value().e0.w << ", " << literal->value().e1.w << ", " << literal->value().e2.w << ", " << literal->value().e3.w << "]";
#define Mat4x4_EXPECTED         "\n[" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ", " << expectedValue_.e2.x << ", " << expectedValue_.e3.x << ";\n" \
                             << " "   << expectedValue_.e0.y << ", " << expectedValue_.e1.y << ", " << expectedValue_.e2.y << ", " << expectedValue_.e3.y << ";\n" \
                             << " "   << expectedValue_.e0.z << ", " << expectedValue_.e1.z << ", " << expectedValue_.e2.z << ", " << expectedValue_.e3.z << ";\n" \
                             << " "   << expectedValue_.e0.w << ", " << expectedValue_.e1.w << ", " << expectedValue_.e2.w << ", " << expectedValue_.e3.w << "]";

#define Quat_COMPARE            literal->value().i == expectedValue_.i && literal->value().j == expectedValue_.j && literal->value().k == expectedValue_.k && literal->value().r == expectedValue_.r
#define Quat_VALUE              literal->value().r << " + " << literal->value().i << "i + " << literal->value().j << "j + " << literal->value().k << "k"
#define Quat_EXPECTED           expectedValue_.r << " + " << expectedValue_.i << "i + " << expectedValue_.j << "j + " << expectedValue_.k << "k"

#define Vec2_COMPARE            literal->value().x == expectedValue_.x && literal->value().y == expectedValue_.y
#define Vec2_VALUE              "[" << literal->value().x << ", " << literal->value().y << "]"
#define Vec2_EXPECTED           "[" << expectedValue_.x << ", " << expectedValue_.y << "]"

#define Vec3_COMPARE            literal->value().x == expectedValue_.x && literal->value().y == expectedValue_.y && literal->value().z == expectedValue_.z
#define Vec3_VALUE              "[" << literal->value().x << ", " << literal->value().y << ", " << literal->value().z << "]"
#define Vec3_EXPECTED           "[" << expectedValue_.x << ", " << expectedValue_.y << ", " << expectedValue_.z << "]"

#define Vec4_COMPARE            literal->value().x == expectedValue_.x && literal->value().y == expectedValue_.y && literal->value().z == expectedValue_.z && literal->value().w == expectedValue_.w
#define Vec4_VALUE              "[" << literal->value().x << ", " << literal->value().y << ", " << literal->value().z << ", " << literal->value().w << "]"
#define Vec4_EXPECTED           "[" << expectedValue_.x << ", " << expectedValue_.y << ", " << expectedValue_.z << ", " << expectedValue_.w << "]"

#define X(dbname, cppname)                                                    \
dbname##LiteralEqMatcher::dbname##LiteralEqMatcher(const cppname& expectedValue) \
    : expectedValue_(expectedValue)                                           \
{}                                                                            \
                                                                              \
bool dbname##LiteralEqMatcher::MatchAndExplain(const odb::ast::dbname##Literal* literal, testing::MatchResultListener* listener) const \
{                                                                             \
    *listener << "literal->value() equals " << dbname##_VALUE;                \
    return dbname##_COMPARE;                                                  \
}                                                                             \
                                                                              \
void dbname##LiteralEqMatcher::DescribeTo(::std::ostream* os) const           \
{                                                                             \
    *os << "literal->value() equals " << dbname##_EXPECTED;                   \
}                                                                             \
                                                                              \
void dbname##LiteralEqMatcher::DescribeNegationTo(::std::ostream* os) const   \
{                                                                             \
    *os << "literal->value() does not equal " << dbname##_EXPECTED;           \
}
ODB_DATATYPE_LIST
#undef X
