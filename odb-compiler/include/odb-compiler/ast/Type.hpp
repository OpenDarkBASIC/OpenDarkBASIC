#pragma once

#include <variant>
#include <memory>
#include <optional>

#include "odb-compiler/ast/Annotation.hpp"
#include "odb-compiler/commands/Command.hpp"
#include "odb-compiler/config.hpp"
#include "odb-sdk/Reference.hpp"

/*!
 * @brief All of the DarkBASIC primitive types that can exist and what types
 * they map to in C++
 */
#define ODB_DATATYPE_LIST_IMPL(X)               \
    X(DoubleInteger, int64_t)                   \
    X(Integer,       int32_t)                   \
    X(Dword,         uint32_t)                  \
    X(Word,          uint16_t)                  \
    X(Byte,          uint8_t)                   \
    X(Boolean,       bool)                      \
    X(DoubleFloat,   double)                    \
    X(Float,         float)                     \
    X(String,        std::string)               \
    X(Complex,       odb::ast::Complex<float>)  \
    X(Mat2x2,        odb::ast::Mat2x2<float>)   \
    X(Mat2x3,        odb::ast::Mat2x3<float>)   \
    X(Mat2x4,        odb::ast::Mat2x4<float>)   \
    X(Mat3x2,        odb::ast::Mat3x2<float>)   \
    X(Mat3x3,        odb::ast::Mat3x3<float>)   \
    X(Mat3x4,        odb::ast::Mat3x4<float>)   \
    X(Mat4x2,        odb::ast::Mat4x2<float>)   \
    X(Mat4x3,        odb::ast::Mat4x3<float>)   \
    X(Mat4x4,        odb::ast::Mat4x4<float>)   \
    X(Quat,          odb::ast::Quat<float>)     \
    X(Vec2,          odb::ast::Vec2<float>)     \
    X(Vec3,          odb::ast::Vec3<float>)     \
    X(Vec4,          odb::ast::Vec4<float>)

#define ODB_DATATYPE_LIST \
    ODB_DATATYPE_LIST_IMPL(X)

namespace odb::ast {

// An enum of builtin datatypes i.e. not user defined types.
enum class BuiltinType
{
#define X(dbname, cppname) dbname,
    ODB_DATATYPE_LIST
#undef X
};

ODBCOMPILER_PUBLIC_API bool isIntegralType(BuiltinType type);
ODBCOMPILER_PUBLIC_API bool isFloatingPointType(BuiltinType type);
ODBCOMPILER_PUBLIC_API const char* builtinTypeEnumString(BuiltinType type);

// Literal types.
template <typename T> struct Complex { T real = 0; T imag = 0; };
template <typename T> struct Quat    { T r = 1; T i = 0; T j = 0; T k = 0; };
template <typename T> struct Vec2    { T x = 0; T y = 0; };
template <typename T> struct Vec3    { T x = 0; T y = 0; T z = 0; };
template <typename T> struct Vec4    { T x = 0; T y = 0; T z = 0; T w = 0; };
template <typename T> struct Mat2x2  { Vec2<T> e0 = {1, 0}; Vec2<T> e1 = {0, 1}; };
template <typename T> struct Mat2x3  { Vec2<T> e0 = {0, 0}; Vec2<T> e1 = {0, 0}; Vec2<T> e2 = {0, 0}; };
template <typename T> struct Mat2x4  { Vec2<T> e0 = {0, 0}; Vec2<T> e1 = {0, 0}; Vec2<T> e2 = {0, 0}; Vec2<T> e3 = {0, 0}; };
template <typename T> struct Mat3x2  { Vec3<T> e0 = {0, 0, 0}; Vec3<T> e1 = {0, 0, 0}; };
template <typename T> struct Mat3x3  { Vec3<T> e0 = {1, 0, 0}; Vec3<T> e1 = {0, 1, 0}; Vec3<T> e2 = {0, 0, 1}; };
template <typename T> struct Mat3x4  { Vec3<T> e0 = {0, 0, 0}; Vec3<T> e1 = {0, 0, 0}; Vec3<T> e2 = {0, 0, 0}; Vec3<T> e3 = {0, 0, 0}; };
template <typename T> struct Mat4x2  { Vec4<T> e0 = {0, 0, 0, 0}; Vec4<T> e1 = {0, 0, 0, 0}; };
template <typename T> struct Mat4x3  { Vec4<T> e0 = {0, 0, 0, 0}; Vec4<T> e1 = {0, 0, 0, 0}; Vec4<T> e2 = {0, 0, 0, 0}; };
template <typename T> struct Mat4x4  { Vec4<T> e0 = {1, 0, 0, 0}; Vec4<T> e1 = {0, 1, 0, 0}; Vec4<T> e2 = {0, 0, 1, 0}; Vec4<T> e3 = {0, 0, 0, 1}; };

// Type trait that maps a C++ type to the corresponding BuiltinType enum.
template <typename T> struct LiteralType
{
};
#define X(dbname, cppname)                                                                                             \
template <> struct LiteralType<cppname>                                                                            \
{                                                                                                                  \
    static constexpr BuiltinType type = BuiltinType::dbname;                                                       \
};
ODB_DATATYPE_LIST
#undef X

// Describes a type in the DBP type system.
class ODBCOMPILER_PUBLIC_API Type
{
public:
    static Type getVoid();
    static Type getBuiltin(BuiltinType builtin);
    static Type getUDT(std::string name);
    static Type getArray(Type inner);
    static Type getFromAnnotation(ast::Annotation annotation);
    static Type getFromCommandType(cmd::Command::Type commandType);

    bool isVoid() const;
    bool isBuiltinType() const;
    bool isUDT() const;
    bool isArray() const;

    size_t size() const;

    std::optional<std::string> getUDT() const;
    std::optional<BuiltinType> getBuiltinType() const;
    std::optional<Type> getArrayInnerType() const;

    std::string toString() const;

    bool isConvertibleTo(Type other) const;

    bool operator==(const Type& other) const;
    bool operator!=(const Type& other) const;

private:
    struct VoidType {};
    struct UDTType { std::string udt; };
    struct ArrayType
    {
        explicit ArrayType(Type type);
        ArrayType(const ArrayType& other);
        ArrayType& operator=(const ArrayType& other);

        std::unique_ptr<Type> inner;
    };

    using TypeVariant = std::variant<VoidType, BuiltinType, UDTType, ArrayType>;

    explicit Type(TypeVariant variant);

    TypeVariant variant_;
};

}
