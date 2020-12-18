#include "odb-compiler/ast/ArrayRef.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/ExpressionList.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
ArrayRef::ArrayRef(AnnotatedSymbol* symbol, ExpressionList* args, SourceLocation* location) :
    Expression(location),
    symbol_(symbol),
    args_(args)
{
    symbol->setParent(this);
    args->setParent(this);
}

// ----------------------------------------------------------------------------
AnnotatedSymbol* ArrayRef::symbol() const
{
    return symbol_;
}

// ----------------------------------------------------------------------------
ExpressionList* ArrayRef::args() const
{
    return args_;
}

// ----------------------------------------------------------------------------
void ArrayRef::accept(Visitor* visitor) const
{
    visitor->visitArrayRef(this);
    symbol_->accept(visitor);
    args_->accept(visitor);
}

}
}

