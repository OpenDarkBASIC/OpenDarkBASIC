#include "odb-compiler/ast/Label.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/Subroutine.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
SubCallSymbol::SubCallSymbol(Symbol* label, SourceLocation* location) :
    Statement(location),
    label_(label)
{
    label->setParent(this);
}

// ----------------------------------------------------------------------------
Symbol* SubCallSymbol::labelSymbol() const
{
    return label_;
}

// ----------------------------------------------------------------------------
void SubCallSymbol::accept(Visitor* visitor)
{
    visitor->visitSubCallSymbol(this);
    label_->accept(visitor);
}
void SubCallSymbol::accept(ConstVisitor* visitor) const
{
    visitor->visitSubCallSymbol(this);
    label_->accept(visitor);
}

// ----------------------------------------------------------------------------
void SubCallSymbol::swapChild(const Node* oldNode, Node* newNode)
{
    if (label_ == oldNode)
        label_ = dynamic_cast<Symbol*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* SubCallSymbol::duplicateImpl() const
{
    return new SubCallSymbol(
        label_->duplicate<Symbol>(),
        location());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
SubCall::SubCall(Label* label, SourceLocation* location) :
    Statement(location),
    label_(label)
{
    label->setParent(this);
}

// ----------------------------------------------------------------------------
Label* SubCall::label() const
{
    return label_;
}

// ----------------------------------------------------------------------------
void SubCall::accept(Visitor* visitor)
{
    visitor->visitSubCall(this);
    label_->accept(visitor);
}
void SubCall::accept(ConstVisitor* visitor) const
{
    visitor->visitSubCall(this);
    label_->accept(visitor);
}

// ----------------------------------------------------------------------------
void SubCall::swapChild(const Node* oldNode, Node* newNode)
{
    if (label_ == oldNode)
        label_ = dynamic_cast<Label*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* SubCall::duplicateImpl() const
{
    return new SubCall(
        label_->duplicate<Label>(),
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
void SubReturn::accept(Visitor* visitor)
{
    visitor->visitSubReturn(this);
}
void SubReturn::accept(ConstVisitor* visitor) const
{
    visitor->visitSubReturn(this);
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
}
