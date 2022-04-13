#include "odb-compiler/ast/VarDecl.hpp"
#include "odb-compiler/ast/InitializerList.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/ScopedAnnotatedSymbol.hpp"
#include "odb-compiler/ast/UDTRef.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {
namespace {
InitializerList* defaultInitializer(const Type& type, SourceLocation* location)
{
    Expression* defaultLiteral = nullptr;
    if (type.isBuiltinType()) {
        switch (*type.getBuiltinType())
        {
#define X(dbname, cppname)                                             \
        case BuiltinType::dbname:                                      \
            defaultLiteral = new dbname##Literal(cppname(), location); \
            break;
            ODB_DATATYPE_LIST
#undef X
            default:
                return nullptr;
        }
        return new InitializerList(defaultLiteral, location);
    }
    else
    {
        return nullptr;
    }
}
}

// ----------------------------------------------------------------------------
VarDecl::VarDecl(ScopedAnnotatedSymbol* symbol, Type type, InitializerList* initializer, SourceLocation* location) :
    Statement(location),
    symbol_(symbol),
    type_(std::move(type)),
    initializer_(initializer)
{
    symbol->setParent(this);
    if (type_.isUDT())
    {
        (*type_.getUDT())->setParent(this);
    }
    if (initializer)
    {
        initializer->setParent(this);
    }
}

// ----------------------------------------------------------------------------
VarDecl::VarDecl(ScopedAnnotatedSymbol* symbol, Type type, SourceLocation* location) :
    VarDecl(symbol, std::move(type), defaultInitializer(type, location), location)
{
}

// ----------------------------------------------------------------------------
ScopedAnnotatedSymbol* VarDecl::symbol() const
{
    return symbol_;
}

// ----------------------------------------------------------------------------
Type VarDecl::type() const
{
    return type_;
}

// ----------------------------------------------------------------------------
MaybeNull<InitializerList> VarDecl::initializer() const
{
    return initializer_.get();
}

// ----------------------------------------------------------------------------
void VarDecl::setInitializer(InitializerList* initializer)
{
    initializer_ = initializer;
    initializer->setParent(this);
}

// ----------------------------------------------------------------------------
std::string VarDecl::toString() const
{
    return "VarDecl";
}

// ----------------------------------------------------------------------------
void VarDecl::accept(Visitor* visitor)
{
    visitor->visitVarDecl(this);
    symbol_->accept(visitor);
    if (type_.isUDT())
    {
        (*type_.getUDT())->accept(visitor);
    }
    if (initializer_)
    {
        initializer_->accept(visitor);
    }
}

// ----------------------------------------------------------------------------
void VarDecl::accept(ConstVisitor* visitor) const
{
    visitor->visitVarDecl(this);
    symbol_->accept(visitor);
    if (type_.isUDT())
    {
        (*type_.getUDT())->accept(visitor);
    }
    if (initializer_)
    {
        initializer_->accept(visitor);
    }
}

// ----------------------------------------------------------------------------
void VarDecl::swapChild(const Node* oldNode, Node* newNode)
{
    if (symbol_ == oldNode)
        symbol_ = dynamic_cast<ScopedAnnotatedSymbol*>(newNode);
    else if (type_.isUDT() && *type_.getUDT() == oldNode)
        type_ = Type::getUDT(dynamic_cast<UDTRef*>(newNode));
    else if (initializer_ == oldNode)
        initializer_ = dynamic_cast<InitializerList*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* VarDecl::duplicateImpl() const
{
    return new VarDecl(
        symbol_->duplicate<ScopedAnnotatedSymbol>(),
        type_.isUDT() ? Type::getUDT((*type_.getUDT())->duplicate<UDTRef>()) : type_,
        initializer_->duplicate<InitializerList>(),
        location());
}

}
