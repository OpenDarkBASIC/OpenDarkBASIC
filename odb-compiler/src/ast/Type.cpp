#include "odb-compiler/ast/Type.hpp"

namespace odb::ast
{
bool isIntegralType(BuiltinType type)
{
    return type == BuiltinType::DoubleInteger || type == BuiltinType::Integer || type == BuiltinType::Dword ||
           type == BuiltinType::Word || type == BuiltinType::Byte || type == BuiltinType::Boolean;
}

bool isFloatingPointType(BuiltinType type)
{
    return type == BuiltinType::DoubleFloat || type == BuiltinType::Float;
}

const char* builtinTypeEnumString(BuiltinType type)
{
    switch (type)
    {
#define X(dbname, cppname)                                                                                             \
case BuiltinType::dbname:                                                                                          \
    return #dbname;
        ODB_DATATYPE_LIST
#undef X
        default:
            return "";
    }
}

Type Type::getVoid() {
    return Type(VoidType{});
}

Type Type::getBuiltin(BuiltinType builtin) {
    return Type(builtin);
}

Type Type::getUDT(UDTDecl* udt) {
    return Type(UDTType{udt});
}

Type Type::getArray(Type inner) {
    return Type(ArrayType{inner});
}

bool Type::isVoid() const
{
    return std::holds_alternative<VoidType>(variant_);
}

bool Type::isBuiltinType() const
{
    return std::holds_alternative<BuiltinType>(variant_);
}

bool Type::isUDT() const
{
    return std::holds_alternative<UDTType>(variant_);
}

bool Type::isArray() const
{
    return std::holds_alternative<ArrayType>(variant_);
}

size_t Type::size() const
{
    if (isVoid())
    {
        return 0;
    }
    else if (isUDT() || isArray())
    {
        // Pointer size.
        return 4;
    }
    else
    {
        switch (*std::get_if<BuiltinType>(&variant_))
        {
            case BuiltinType::DoubleInteger:
                return 8;
            case BuiltinType::Integer:
                return 4;
            case BuiltinType::Dword:
                return 4;
            case BuiltinType::Word:
                return 2;
            case BuiltinType::Byte:
                return 1;
            case BuiltinType::Boolean:
                return 4;
            case BuiltinType::DoubleFloat:
                return 8;
            case BuiltinType::Float:
                return 4;
            case BuiltinType::String:
                return 4; // TODO: pointer type?
            case BuiltinType::Complex:
                return 4*2;
            case BuiltinType::Mat2x2:
                return 4*2*2;
            case BuiltinType::Mat2x3:
                return 4*2*3;
            case BuiltinType::Mat2x4:
                return 4*2*4;
            case BuiltinType::Mat3x2:
                return 4*3*2;
            case BuiltinType::Mat3x3:
                return 4*3*3;
            case BuiltinType::Mat3x4:
                return 4*3*4;
            case BuiltinType::Mat4x2:
                return 4*4*2;
            case BuiltinType::Mat4x3:
                return 4*4*3;
            case BuiltinType::Mat4x4:
                return 4*4*4;
            case BuiltinType::Quat:
                return 4*4;
            case BuiltinType::Vec2:
                return 4*2;
            case BuiltinType::Vec3:
                return 4*3;
            case BuiltinType::Vec4:
                return 4*4;
        }
    }
}

std::optional<UDTDecl*> Type::getUDT() const
{
    return isUDT() ? std::optional<UDTDecl*>{std::get_if<UDTType>(&variant_)->udt} : std::nullopt;
}

std::optional<BuiltinType> Type::getBuiltinType() const
{
    return isBuiltinType() ? std::optional<BuiltinType>{*std::get_if<BuiltinType>(&variant_)} : std::nullopt;
}

std::optional<Type> Type::getArrayInnerType() const
{
    return isArray() ? std::optional<Type>{*std::get_if<ArrayType>(&variant_)->inner} : std::nullopt;
}

bool Type::operator==(const Type& other) const
{
    // Are the two types different?
    if (variant_.index() != other.variant_.index())
    {
        return false;
    }
    else if (isVoid())
    {
        return true;
    }
    else if (isUDT())
    {
        return false; // udt_->get == other.udt_->name;
    }
    else if (isArray())
    {
        return *std::get_if<ArrayType>(&variant_)->inner == *std::get_if<ArrayType>(&other.variant_)->inner;
    }
    else
    {
        return *std::get_if<BuiltinType>(&variant_) == *std::get_if<BuiltinType>(&other.variant_);
    }
}

bool Type::operator!=(const Type& other) const
{
    return !(*this == other);
}

std::string Type::toString() const
{
    if (isBuiltinType())
    {
        return builtinTypeEnumString(*std::get_if<BuiltinType>(&variant_));
    }
    else if (isArray())
    {
        return "Array<" + std::get_if<ArrayType>(&variant_)->inner->toString() + ">";
    }
    else if (isUDT())
    {
        return "!UNIMPLEMENTED UDT TYPE!";
    }
    else
    {
        return "void";
    }
}

Type::ArrayType::ArrayType(Type inner) : inner(std::make_unique<Type>(inner)) {
}

Type::ArrayType::ArrayType(const Type::ArrayType& other) : inner(std::make_unique<Type>(*other.inner))
{
}

Type::ArrayType& Type::ArrayType::operator=(const Type::ArrayType& other) {
    inner = std::make_unique<Type>(*other.inner);
    return *this;
}

Type::Type(Type::TypeVariant variant) : variant_(std::move(variant))
{
}
}