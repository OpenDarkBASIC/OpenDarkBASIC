#include "odb-compiler/ast/ArrayDecl.hpp"
#include "odb-compiler/ast/ArgList.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/ScopedIdentifier.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
ArrayDecl::ArrayDecl(ScopedIdentifier* identifier, Type type, ArgList* dims, SourceLocation* location)
    : Statement(location)
    , identifier_(identifier)
    , type_(std::move(type))
    , dims_(dims)
{
}

// ----------------------------------------------------------------------------
ScopedIdentifier* ArrayDecl::identifier() const
{
    return identifier_;
}

// ----------------------------------------------------------------------------
Type ArrayDecl::type() const
{
    return type_;
}

// ----------------------------------------------------------------------------
ArgList* ArrayDecl::dims() const
{
    return dims_;
}

// ----------------------------------------------------------------------------
std::string ArrayDecl::toString() const
{
    return "ArrayDecl";
}

// ----------------------------------------------------------------------------
void ArrayDecl::accept(Visitor* visitor)
{
    visitor->visitArrayDecl(this);
}

// ----------------------------------------------------------------------------
void ArrayDecl::accept(ConstVisitor* visitor) const
{
    visitor->visitArrayDecl(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange ArrayDecl::children()
{
    Node::ChildRange children;
    children.push_back(identifier_);
    children.push_back(dims_);
    return children;
}

// ----------------------------------------------------------------------------
void ArrayDecl::swapChild(const Node* oldNode, Node* newNode)
{
    if (identifier_ == oldNode)
        identifier_ = dynamic_cast<ScopedIdentifier*>(newNode);
    else if (dims_ == oldNode)
        dims_ = dynamic_cast<ArgList*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* ArrayDecl::duplicateImpl() const
{
    return new ArrayDecl(
        identifier_->duplicate<ScopedIdentifier>(),
        type_,
        dims_->duplicate<ArgList>(),
        location());
}

}
