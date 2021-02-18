#include "odb-compiler/ast/Label.hpp"
#include "odb-compiler/ast/Goto.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
GotoSymbol::GotoSymbol(Symbol* label, SourceLocation* location) :
    Statement(location),
    label_(label)
{
    label->setParent(this);
}

// ----------------------------------------------------------------------------
Symbol* GotoSymbol::labelSymbol() const
{
    return label_;
}

// ----------------------------------------------------------------------------
void GotoSymbol::accept(Visitor* visitor)
{
    visitor->visitGotoSymbol(this);
    label_->accept(visitor);
}
void GotoSymbol::accept(ConstVisitor* visitor) const
{
    visitor->visitGotoSymbol(this);
    label_->accept(visitor);
}

// ----------------------------------------------------------------------------
void GotoSymbol::swapChild(const Node* oldNode, Node* newNode)
{
    if (label_ == oldNode)
        label_ = dynamic_cast<Symbol*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* GotoSymbol::duplicateImpl() const
{
    return new GotoSymbol(
        label_->duplicate<Symbol>(),
        location());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
Goto::Goto(Label* label, SourceLocation* location) :
    Statement(location),
    label_(label)
{
    // DON'T set parent. We're only holding a weak reference
}

// ----------------------------------------------------------------------------
WeakReference<Label> Goto::label() const
{
    return label_;
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
void Goto::swapChild(const Node* oldNode, Node* newNode)
{
    if (label_ == oldNode)
        label_ = dynamic_cast<Label*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* Goto::duplicateImpl() const
{
    // Goto node can't be duplicated because it references another node in the
    // old tree
    assert(false);
    return nullptr;
}

}
}
