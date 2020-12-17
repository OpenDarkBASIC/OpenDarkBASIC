#include "odb-compiler/ast/ConstDecl.hpp"
#include "odb-compiler/ast/AnnotatedSymbol.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
ConstDecl::ConstDecl(AnnotatedSymbol* symbol, Literal* literal, SourceLocation* location) :
    Statement(location),
    symbol_(symbol),
    literal_(literal)
{
    symbol->setParent(this);
    literal->setParent(this);
}

// ----------------------------------------------------------------------------
AnnotatedSymbol* ConstDecl::symbol() const
{
    return symbol_;
}

// ----------------------------------------------------------------------------
Literal* ConstDecl::literal() const
{
    return literal_;
}

// ----------------------------------------------------------------------------
void ConstDecl::accept(Visitor* visitor) const
{
    visitor->visitConstDecl(this);
    symbol_->accept(visitor);
    literal_->accept(visitor);
}

}
}
