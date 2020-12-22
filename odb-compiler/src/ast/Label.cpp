#include "odb-compiler/ast/Label.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb {
namespace ast {

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
void Label::accept(Visitor* visitor) const
{
    visitor->visitLabel(this);
    symbol_->accept(visitor);
}

}
}
