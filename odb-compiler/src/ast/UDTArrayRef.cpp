#include "odb-compiler/ast/ExpressionList.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/UDTArrayRef.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
UDTArrayRef::UDTArrayRef(Symbol* symbol, ExpressionList* dims, LValue* fieldRef, SourceLocation* location)
    : LValue(location)
    , symbol_(symbol)
    , dims_(dims)
    , field_(fieldRef)
{
    symbol->setParent(this);
    dims->setParent(this);
    fieldRef->setParent(this);
}

// ----------------------------------------------------------------------------
Symbol* UDTArrayRef::symbol() const
{
    return symbol_;
}

// ----------------------------------------------------------------------------
ExpressionList* UDTArrayRef::dims() const
{
    return dims_;
}

// ----------------------------------------------------------------------------
LValue* UDTArrayRef::fieldRef() const
{
    return field_;
}

// ----------------------------------------------------------------------------
void UDTArrayRef::accept(Visitor* visitor)
{
    visitor->visitUDTArrayRef(this);
    symbol_->accept(visitor);
    dims_->accept(visitor);
    field_->accept(visitor);
}

// ----------------------------------------------------------------------------
void UDTArrayRef::accept(ConstVisitor* visitor) const
{
    visitor->visitUDTArrayRef(this);
    symbol_->accept(visitor);
    dims_->accept(visitor);
    field_->accept(visitor);
}

// ----------------------------------------------------------------------------
void UDTArrayRef::swapChild(const Node* oldNode, Node* newNode)
{
    if (symbol_ == oldNode)
        symbol_ = dynamic_cast<Symbol*>(newNode);
    else if (dims_ == oldNode)
        dims_ = dynamic_cast<ExpressionList*>(newNode);
    else if (field_ == oldNode)
        field_ = dynamic_cast<LValue*>(newNode);
    else
        assert(false);
}

}
