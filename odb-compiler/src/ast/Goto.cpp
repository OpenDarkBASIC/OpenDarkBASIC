#include "odb-compiler/ast/Label.hpp"
#include "odb-compiler/ast/Goto.hpp"
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
void GotoSymbol::accept(Visitor* visitor) const
{
    visitor->visitGotoSymbol(this);
    label_->accept(visitor);
}

// ----------------------------------------------------------------------------
Goto::Goto(Label* label, SourceLocation* location) :
    Statement(location),
    label_(label)
{
    label->setParent(this);
}

// ----------------------------------------------------------------------------
Label* Goto::label() const
{
    return label_;
}

// ----------------------------------------------------------------------------
void Goto::accept(Visitor* visitor) const
{
    visitor->visitGoto(this);
    label_->accept(visitor);
}

}
}
