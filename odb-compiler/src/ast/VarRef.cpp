#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/ast/AnnotatedSymbol.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
VarRef::VarRef(AnnotatedSymbol* symbol, SourceLocation* location) :
    Expression(location),
    symbol_(symbol)
{
    symbol->setParent(this);
}
AnnotatedSymbol* VarRef::symbol() const
{
    return symbol_;
}
void VarRef::accept(Visitor* visitor) const
{
    visitor->visitVarRef(this);
    symbol_->accept(visitor);
}

}
}
