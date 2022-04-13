#include "odb-compiler/ast/Label.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
Label::Label(Symbol* symbol, SourceLocation* location) :
    Statement(location),
    symbol_(symbol)
{
    symbol->setParent(this);
}

// ----------------------------------------------------------------------------
Symbol* Label::symbol() const
{
    return symbol_;
}

// ----------------------------------------------------------------------------
std::string Label::toString() const
{
    return "Label";
}

// ----------------------------------------------------------------------------
void Label::accept(Visitor* visitor)
{
    visitor->visitLabel(this);
    symbol_->accept(visitor);
}
void Label::accept(ConstVisitor* visitor) const
{
    visitor->visitLabel(this);
    symbol_->accept(visitor);
}

// ----------------------------------------------------------------------------
void Label::swapChild(const Node* oldNode, Node* newNode)
{
    if (symbol_ == oldNode)
        symbol_ = dynamic_cast<Symbol*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* Label::duplicateImpl() const
{
    return new Label(
        symbol_->duplicate<Symbol>(),
        location());
}

}
