#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/UDTVarRef.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
UDTVarRef::UDTVarRef(Symbol* symbol, LValue* fieldRef, SourceLocation* location)
    : LValue(location)
    , symbol_(symbol)
    , field_(fieldRef)
{
    symbol->setParent(this);
    fieldRef->setParent(this);
}

// ----------------------------------------------------------------------------
Symbol* UDTVarRef::symbol() const
{
    return symbol_;
}

// ----------------------------------------------------------------------------
LValue* UDTVarRef::fieldRef() const
{
    return field_;
}

// ----------------------------------------------------------------------------
void UDTVarRef::accept(Visitor* visitor)
{
    visitor->visitUDTVarRef(this);
    symbol_->accept(visitor);
    field_->accept(visitor);
}

// ----------------------------------------------------------------------------
void UDTVarRef::accept(ConstVisitor* visitor) const
{
    visitor->visitUDTVarRef(this);
    symbol_->accept(visitor);
    field_->accept(visitor);
}

// ----------------------------------------------------------------------------
void UDTVarRef::swapChild(const Node* oldNode, Node* newNode)
{
    if (symbol_ == oldNode)
        symbol_ = dynamic_cast<Symbol*>(newNode);
    else if (field_ == oldNode)
        field_ = dynamic_cast<LValue*>(newNode);
    else
        assert(false);
}

}
