#include "odb-compiler/ast/Label.hpp"
#include "odb-compiler/ast/Goto.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Identifier.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
UnresolvedGoto::UnresolvedGoto(Identifier* label, SourceLocation* location) :
    Statement(location),
    label_(label)
{
}

// ----------------------------------------------------------------------------
Identifier* UnresolvedGoto::label() const
{
    return label_;
}

// ----------------------------------------------------------------------------
std::string UnresolvedGoto::toString() const
{
    return "UnresolvedGoto";
}

// ----------------------------------------------------------------------------
void UnresolvedGoto::accept(Visitor* visitor)
{
    visitor->visitUnresolvedGoto(this);
}
void UnresolvedGoto::accept(ConstVisitor* visitor) const
{
    visitor->visitUnresolvedGoto(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange UnresolvedGoto::children()
{
    return {label_};
}

// ----------------------------------------------------------------------------
void UnresolvedGoto::swapChild(const Node* oldNode, Node* newNode)
{
    if (label_ == oldNode)
        label_ = dynamic_cast<Identifier*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* UnresolvedGoto::duplicateImpl() const
{
    return new UnresolvedGoto(
        label_->duplicate<Identifier>(),
        location());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
Goto::Goto(Label* label, SourceLocation* location) :
    Statement(location),
    label_(label)
{
}

// ----------------------------------------------------------------------------
Label* Goto::label() const
{
    return label_;
}

// ----------------------------------------------------------------------------
std::string Goto::toString() const
{
    return "Goto";
}

// ----------------------------------------------------------------------------
void Goto::accept(Visitor* visitor)
{
    visitor->visitGoto(this);
}
void Goto::accept(ConstVisitor* visitor) const
{
    visitor->visitGoto(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange Goto::children()
{
    // The label is not a child, to avoid creating a cycle in the AST.
    return {};
}

// ----------------------------------------------------------------------------
void Goto::swapChild(const Node* oldNode, Node* newNode)
{
    assert(false);
}

// ----------------------------------------------------------------------------
Node* Goto::duplicateImpl() const
{
    return new Goto(
        label_,
        location());
}


}
