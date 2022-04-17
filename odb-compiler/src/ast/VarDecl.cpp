#include "odb-compiler/ast/VarDecl.hpp"
#include "odb-compiler/ast/InitializerList.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/ScopedIdentifier.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
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
VarDecl::VarDecl(ScopedIdentifier* identifier, Type type, InitializerList* initializer, SourceLocation* location) :
    Statement(location),
    identifier_(identifier),
    type_(std::move(type)),
    initializer_(initializer)
{
}

// ----------------------------------------------------------------------------
VarDecl::VarDecl(ScopedIdentifier* identifier, Type type, SourceLocation* location) :
    VarDecl(identifier, std::move(type), defaultInitializer(type, location), location)
{
}

// ----------------------------------------------------------------------------
ScopedIdentifier* VarDecl::identifier() const
{
    return identifier_;
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
}

// ----------------------------------------------------------------------------
void VarDecl::accept(ConstVisitor* visitor) const
{
    visitor->visitVarDecl(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange VarDecl::children()
{
    ChildRange children;
    children.push_back(identifier_);
    if (initializer_)
    {
        children.push_back(initializer_);
    }
    return children;
}

// ----------------------------------------------------------------------------
void VarDecl::swapChild(const Node* oldNode, Node* newNode)
{
    if (identifier_ == oldNode)
        identifier_ = dynamic_cast<ScopedIdentifier*>(newNode);
    else if (initializer_ == oldNode)
        initializer_ = dynamic_cast<InitializerList*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* VarDecl::duplicateImpl() const
{
    return new VarDecl(
        identifier_->duplicate<ScopedIdentifier>(),
        type_,
        initializer_->duplicate<InitializerList>(),
        location());
}

}
