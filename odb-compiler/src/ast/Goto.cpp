#include "odb-compiler/ast/Label.hpp"
#include "odb-compiler/ast/Goto.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
Goto::Goto(Symbol* label, SourceLocation* location) :
    Statement(location),
    label_(label)
{
    label->setParent(this);
}

// ----------------------------------------------------------------------------
Symbol* Goto::label() const
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
    return {label_};
}

// ----------------------------------------------------------------------------
void Goto::swapChild(const Node* oldNode, Node* newNode)
{
    if (label_ == oldNode)
        label_ = dynamic_cast<Symbol*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* Goto::duplicateImpl() const
{
    return new Goto(
        label_->duplicate<Symbol>(),
        location());
}

}
