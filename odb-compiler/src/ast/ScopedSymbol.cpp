#include "odb-compiler/ast/ScopedSymbol.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
ScopedSymbol::ScopedSymbol(Scope scope, const std::string& name, SourceLocation* location) :
    Symbol(name, location),
    scope_(scope)
{
}

// ----------------------------------------------------------------------------
Symbol::Scope ScopedSymbol::scope() const
{
    return scope_;
}

// ----------------------------------------------------------------------------
void ScopedSymbol::accept(Visitor* visitor) const
{
    visitor->visitScopedSymbol(this);
}

}
}
