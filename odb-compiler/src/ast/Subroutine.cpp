#include "odb-compiler/ast/Label.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/Subroutine.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
SubCall::SubCall(Symbol* label, SourceLocation* location) :
    Statement(location),
    label_(label)
{
    label->setParent(this);
}

// ----------------------------------------------------------------------------
Symbol* SubCall::label() const
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
    return {label_};
}

// ----------------------------------------------------------------------------
void SubCall::swapChild(const Node* oldNode, Node* newNode)
{
    if (label_ == oldNode)
        label_ = dynamic_cast<Symbol*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* SubCall::duplicateImpl() const
{
    return new SubCall(
        label_->duplicate<Symbol>(),
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
