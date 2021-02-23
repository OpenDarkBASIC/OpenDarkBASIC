#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
VarRef::VarRef(AnnotatedSymbol* symbol, SourceLocation* location) :
    LValue(location),
    symbol_(symbol)
{
    symbol->setParent(this);
}

// ----------------------------------------------------------------------------
AnnotatedSymbol* VarRef::symbol() const
{
    return symbol_;
}

// ----------------------------------------------------------------------------
std::string VarRef::toString() const
{
    return "VarRef";
}

// ----------------------------------------------------------------------------
void VarRef::accept(Visitor* visitor)
{
    visitor->visitVarRef(this);
    symbol_->accept(visitor);
}
void VarRef::accept(ConstVisitor* visitor) const
{
    visitor->visitVarRef(this);
    symbol_->accept(visitor);
}

// ----------------------------------------------------------------------------
void VarRef::swapChild(const Node* oldNode, Node* newNode)
{
    if (symbol_ == oldNode)
        symbol_ = dynamic_cast<AnnotatedSymbol*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* VarRef::duplicateImpl() const
{
    return new VarRef(
        symbol_->duplicate<AnnotatedSymbol>(),
        location());
}

}
