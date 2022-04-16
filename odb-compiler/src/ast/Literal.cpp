#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"
#include "odb-sdk/Str.hpp"

// These macros get pasted into the body of toString() of each type
#define DoubleInteger_STR "DoubleInteger: " + std::to_string(value_)
#define Integer_STR       "Integer: " + std::to_string(value_)
#define Dword_STR         "Dword: " + std::to_string(value_)
#define Word_STR          "Word: " + std::to_string(value_)
#define Byte_STR          "Byte: " + std::to_string(value_)
#define Boolean_STR       std::string("Boolean: ") + (value_ ? "true" : "false")
#define DoubleFloat_STR   "DoubleFloat: " + std::to_string(value_)
#define Float_STR         "Float: " + std::to_string(value_)
#define String_STR        "String: \"" + str::escape(value_) + "\""
#define Complex_STR       "Complex: " + std::to_string(value_.real) + " + " + std::to_string(value_.imag) + "i"
#define Quat_STR          "Quat: " + std::to_string(value_.r) + " + " + std::to_string(value_.i) + "i + " + std::to_string(value_.j) + "j + " + std::to_string(value_.k) + "k"
#define Vec2_STR          "Vec2: [" + std::to_string(value_.x) + ", " + std::to_string(value_.y) + "]"
#define Vec3_STR          "Vec3: [" + std::to_string(value_.x) + ", " + std::to_string(value_.y) + ", " + std::to_string(value_.z) + "]"
#define Vec4_STR          "Vec4: [" + std::to_string(value_.x) + ", " + std::to_string(value_.y) + ", " + std::to_string(value_.z) + std::to_string(value_.w) + "]"
#define Mat2x2_STR        "Mat2x2: [" + std::to_string(value_.e0.x) + ", " + std::to_string(value_.e1.x) + ";\n" \
                        + "         " + std::to_string(value_.e0.y) + ", " + std::to_string(value_.e1.y) + "]"
#define Mat2x3_STR        "Mat2x3: [" + std::to_string(value_.e0.x) + ", " + std::to_string(value_.e1.x) + ", " + std::to_string(value_.e2.x) + ";\n" \
                        + "         " + std::to_string(value_.e0.y) + ", " + std::to_string(value_.e1.y) + ", " + std::to_string(value_.e2.y) + "]"
#define Mat2x4_STR        "Mat2x4: [" + std::to_string(value_.e0.x) + ", " + std::to_string(value_.e1.x) + ", " + std::to_string(value_.e2.x) + ", " + std::to_string(value_.e3.x) + ";\n" \
                        + "         " + std::to_string(value_.e0.y) + ", " + std::to_string(value_.e1.y) + ", " + std::to_string(value_.e2.y) + ", " + std::to_string(value_.e3.y) + "]"
#define Mat3x2_STR        "Mat3x2: [" + std::to_string(value_.e0.x) + ", " + std::to_string(value_.e1.x) + ";\n" \
                        + "         " + std::to_string(value_.e0.y) + ", " + std::to_string(value_.e1.y) + ";\n" \
                        + "         " + std::to_string(value_.e0.z) + ", " + std::to_string(value_.e1.z) + "]"
#define Mat3x3_STR        "Mat3x3: [" + std::to_string(value_.e0.x) + ", " + std::to_string(value_.e1.x) + ", " + std::to_string(value_.e2.x) + ";\n" \
                        + "         " + std::to_string(value_.e0.y) + ", " + std::to_string(value_.e1.y) + ", " + std::to_string(value_.e2.y) + ";\n" \
                        + "         " + std::to_string(value_.e0.z) + ", " + std::to_string(value_.e1.z) + ", " + std::to_string(value_.e2.z) + "]"
#define Mat3x4_STR        "Mat3x4: [" + std::to_string(value_.e0.x) + ", " + std::to_string(value_.e1.x) + ", " + std::to_string(value_.e2.x) + ", " + std::to_string(value_.e3.x) + ";\n" \
                        + "         " + std::to_string(value_.e0.y) + ", " + std::to_string(value_.e1.y) + ", " + std::to_string(value_.e2.y) + ", " + std::to_string(value_.e3.y) + ";\n" \
                        + "         " + std::to_string(value_.e0.z) + ", " + std::to_string(value_.e1.z) + ", " + std::to_string(value_.e2.z) + ", " + std::to_string(value_.e3.z) + "]"
#define Mat4x2_STR        "Mat4x2: [" + std::to_string(value_.e0.x) + ", " + std::to_string(value_.e1.x) + ";\n" \
                        + "         " + std::to_string(value_.e0.y) + ", " + std::to_string(value_.e1.y) + ";\n" \
                        + "         " + std::to_string(value_.e0.z) + ", " + std::to_string(value_.e1.z) + ";\n" \
                        + "         " + std::to_string(value_.e0.w) + ", " + std::to_string(value_.e1.w) + "]"
#define Mat4x3_STR        "Mat4x3: [" + std::to_string(value_.e0.x) + ", " + std::to_string(value_.e1.x) + ", " + std::to_string(value_.e1.x) + ";\n" \
                        + "         " + std::to_string(value_.e0.y) + ", " + std::to_string(value_.e1.y) + ", " + std::to_string(value_.e1.y) + ";\n" \
                        + "         " + std::to_string(value_.e0.z) + ", " + std::to_string(value_.e1.z) + ", " + std::to_string(value_.e1.z) + ";\n" \
                        + "         " + std::to_string(value_.e0.w) + ", " + std::to_string(value_.e1.w) + ", " + std::to_string(value_.e1.w) + "]"
#define Mat4x4_STR        "Mat4x4: [" + std::to_string(value_.e0.x) + ", " + std::to_string(value_.e1.x) + ", " + std::to_string(value_.e2.x) + ", " + std::to_string(value_.e3.x) + ";\n" \
                        + "         " + std::to_string(value_.e0.y) + ", " + std::to_string(value_.e1.y) + ", " + std::to_string(value_.e2.y) + ", " + std::to_string(value_.e3.y) + ";\n" \
                        + "         " + std::to_string(value_.e0.z) + ", " + std::to_string(value_.e1.z) + ", " + std::to_string(value_.e2.z) + ", " + std::to_string(value_.e3.z) + ";\n" \
                        + "         " + std::to_string(value_.e0.w) + ", " + std::to_string(value_.e1.w) + ", " + std::to_string(value_.e2.w) + ", " + std::to_string(value_.e3.w) + "]"

namespace odb::ast {

// ----------------------------------------------------------------------------
Literal::Literal(SourceLocation* location) :
    Expression(location)
{
}

// ----------------------------------------------------------------------------
#define X(dbname, cppname)                                                    \
    dbname##Literal::dbname##Literal(const cppname& value, SourceLocation* location) \
        : Literal(location)                                                   \
        , value_(value)                                                       \
    {}                                                                        \
                                                                              \
    const cppname& dbname##Literal::value() const                             \
    {                                                                         \
        return value_;                                                        \
    }                                                                         \
                                                                              \
    std::string dbname##Literal::toString() const                             \
    {                                                                         \
        return dbname##_STR;                                                  \
    }                                                                         \
                                                                              \
    void dbname##Literal::accept(Visitor* visitor)                            \
    {                                                                         \
        visitor->visit##dbname##Literal(this);                                \
    }                                                                         \
                                                                              \
    void dbname##Literal::accept(ConstVisitor* visitor) const                 \
    {                                                                         \
        visitor->visit##dbname##Literal(this);                                \
    }                                                                         \
                                                                              \
    Node::ChildRange dbname##Literal::children()                              \
    {                                                                         \
        return {};                                                            \
    }                                                                         \
                                                                              \
    void dbname##Literal::swapChild(const Node* oldNode, Node* newNode)       \
    {                                                                         \
        assert(false);                                                        \
    }                                                                         \
                                                                              \
    Node* dbname##Literal::duplicateImpl() const                              \
    {                                                                         \
        return new dbname##Literal(value_, location());                       \
    }
ODB_DATATYPE_LIST
#undef X
}
