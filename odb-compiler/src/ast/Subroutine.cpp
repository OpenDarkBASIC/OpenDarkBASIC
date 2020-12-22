#include "odb-compiler/ast/Label.hpp"
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
void SubCallSymbol::accept(Visitor* visitor) const
{
    visitor->visitSubCallSymbol(this);
    label_->accept(visitor);
}

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
void SubCall::accept(Visitor* visitor) const
{
    visitor->visitSubCall(this);
    label_->accept(visitor);
}

// ----------------------------------------------------------------------------
SubReturn::SubReturn(SourceLocation* location) :
    Statement(location)
{
}

// ----------------------------------------------------------------------------
void SubReturn::accept(Visitor* visitor) const
{
    visitor->visitSubReturn(this);
}

}
}
