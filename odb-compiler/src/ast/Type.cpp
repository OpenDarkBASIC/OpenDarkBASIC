#include <utility>

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

bool isSigned(BuiltinType type)
{
    return type == BuiltinType::DoubleInteger || type == BuiltinType::Integer || type == BuiltinType::DoubleFloat ||
           type == BuiltinType::Float;
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

Type Type::getUnknown(std::string info)
{
    return Type(UnknownType{std::move(info)});
}

Type Type::getVoid() {
    return Type(VoidType{});
}

Type Type::getBuiltin(BuiltinType builtin) {
    return Type(builtin);
}

Type Type::getUDT(std::string name) {
    return Type(UDTType{std::move(name)});
}

Type Type::getArray(Type inner) {
    return Type(ArrayType{std::move(inner)});
}

Type Type::getFromAnnotation(ast::Annotation annotation)
{
    switch (annotation)
    {
    case ast::Annotation::NONE:
        return Type::getBuiltin(BuiltinType::Integer);
    case ast::Annotation::DOUBLE_INTEGER:
        return Type::getBuiltin(BuiltinType::DoubleInteger);
    case ast::Annotation::WORD:
        return Type::getBuiltin(BuiltinType::Word);
    case ast::Annotation::DOUBLE_FLOAT:
        return Type::getBuiltin(BuiltinType::DoubleFloat);
    case ast::Annotation::STRING:
        return Type::getBuiltin(BuiltinType::String);
    case ast::Annotation::FLOAT:
        return Type::getBuiltin(BuiltinType::Float);
    default:
        assert(false && "getFromAnnotation encountered unknown type.");
    }
}

Type Type::getFromCommandType(cmd::Command::Type commandType)
{
    switch (commandType)
    {
    case cmd::Command::Type::Integer:
        return Type::getBuiltin(BuiltinType::Integer);
    case cmd::Command::Type::Float:
        return Type::getBuiltin(BuiltinType::Float);
    case cmd::Command::Type::String:
        return Type::getBuiltin(BuiltinType::String);
    case cmd::Command::Type::Double:
        return Type::getBuiltin(BuiltinType::DoubleFloat);
    case cmd::Command::Type::Long:
        return Type::getBuiltin(BuiltinType::DoubleInteger);
    case cmd::Command::Type::Dword:
        return Type::getBuiltin(BuiltinType::Dword);
    case cmd::Command::Type::Void:
        return Type::getVoid();
    default:
        return Type::getUnknown(std::string(1, static_cast<char>(commandType)));
    }
}

bool Type::isUnknown() const
{
    return std::holds_alternative<UnknownType>(variant_);
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

std::optional<std::string> Type::getUDT() const
{
    return isUDT() ? std::optional<std::string>{std::get_if<UDTType>(&variant_)->udt} : std::nullopt;
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
        return std::get_if<UDTType>(&variant_)->udt == std::get_if<UDTType>(&other.variant_)->udt;
    }
    else if (isArray())
    {
        return *std::get_if<ArrayType>(&variant_)->inner == *std::get_if<ArrayType>(&other.variant_)->inner;
    }
    else if (isBuiltinType())
    {
        return *std::get_if<BuiltinType>(&variant_) == *std::get_if<BuiltinType>(&other.variant_);
    }
    else
    {
        return false;
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
    else if (isVoid())
    {
        return "void";
    }
    else
    {
        auto info = std::get_if<UnknownType>(&variant_)->info;
        if (info.empty())
        {
            return "unknown";
        }
        else
        {
            return "unknown [" + std::get_if<UnknownType>(&variant_)->info + "]";
        }
    }
}

bool Type::isConvertibleTo(Type other) const
{
    if (*this == other)
    {
        return true;
    }
    if (isBuiltinType() && other.isBuiltinType())
    {
        // int -> int casts.
        if (isIntegralType(*getBuiltinType()) && isIntegralType(*other.getBuiltinType()))
        {
            return true;
        }

        // fp -> fp casts.
        if (isFloatingPointType(*getBuiltinType()) && isFloatingPointType(*other.getBuiltinType()))
        {
            return true;
        }

        // int -> fp casts.
        if (isIntegralType(*getBuiltinType()) && isFloatingPointType(*other.getBuiltinType()))
        {
            return true;
        }

        // fp -> int casts.
        if (isFloatingPointType(*getBuiltinType()) && isIntegralType(*other.getBuiltinType()))
        {
            return true;
        }
    }
    return false;
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