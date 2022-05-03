#include "odb-compiler/ast/Label.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Identifier.hpp"
#include "odb-compiler/ast/Subroutine.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
UnresolvedSubCall::UnresolvedSubCall(Identifier* label, SourceLocation* location) :
    Statement(location),
    label_(label)
{
}

// ----------------------------------------------------------------------------
Identifier* UnresolvedSubCall::label() const
{
    return label_;
}

// ----------------------------------------------------------------------------
std::string UnresolvedSubCall::toString() const
{
    return "UnresolvedSubCall";
}

// ----------------------------------------------------------------------------
void UnresolvedSubCall::accept(Visitor* visitor)
{
    visitor->visitUnresolvedSubCall(this);
}
void UnresolvedSubCall::accept(ConstVisitor* visitor) const
{
    visitor->visitUnresolvedSubCall(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange UnresolvedSubCall::children()
{
    return {label_};
}

// ----------------------------------------------------------------------------
void UnresolvedSubCall::swapChild(const Node* oldNode, Node* newNode)
{
    if (label_ == oldNode)
        label_ = dynamic_cast<Identifier*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* UnresolvedSubCall::duplicateImpl() const
{
    return new UnresolvedSubCall(
        label_->duplicate<Identifier>(),
        location());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
SubCall::SubCall(Label* label, SourceLocation* location) :
    Statement(location),
    label_(label)
{
}

// ----------------------------------------------------------------------------
Label* SubCall::label() const
{
    return label_;
}

// ----------------------------------------------------------------------------
std::string SubCall::toString() const
{
    return "SubCall";
}

// ----------------------------------------------------------------------------
void SubCall::accept(Visitor* visitor)
{
    visitor->visitSubCall(this);
}
void SubCall::accept(ConstVisitor* visitor) const
{
    visitor->visitSubCall(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange SubCall::children()
{
    // The label is not a child, to avoid creating a cycle in the AST.
    return {};
}

// ----------------------------------------------------------------------------
void SubCall::swapChild(const Node* oldNode, Node* newNode)
{
    assert(false);
}

// ----------------------------------------------------------------------------
Node* SubCall::duplicateImpl() const
{
    return new SubCall(
        label_,
        location());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
SubReturn::SubReturn(SourceLocation* location) :
    Statement(location)
{
}

// ----------------------------------------------------------------------------
std::string SubReturn::toString() const
{
    return "SubReturn";
}

// ----------------------------------------------------------------------------
void SubReturn::accept(Visitor* visitor)
{
    visitor->visitSubReturn(this);
}
void SubReturn::accept(ConstVisitor* visitor) const
{
    visitor->visitSubReturn(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange SubReturn::children()
{
    return {};
}

// ----------------------------------------------------------------------------
void SubReturn::swapChild(const Node* oldNode, Node* newNode)
{
    assert(false);
}

// ----------------------------------------------------------------------------
Node* SubReturn::duplicateImpl() const
{
    return new SubReturn(location());
}

}
