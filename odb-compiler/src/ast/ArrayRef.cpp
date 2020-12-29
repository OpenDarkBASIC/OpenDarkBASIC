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
void ArrayRef::accept(Visitor* visitor)
{
    visitor->visitArrayRef(this);
    symbol_->accept(visitor);
    args_->accept(visitor);
}
void ArrayRef::accept(ConstVisitor* visitor) const
{
    visitor->visitArrayRef(this);
    symbol_->accept(visitor);
    args_->accept(visitor);
}

// ----------------------------------------------------------------------------
void ArrayRef::swapChild(const Node* oldNode, Node* newNode)
{
    if (symbol_ == oldNode)
        symbol_ = dynamic_cast<AnnotatedSymbol*>(newNode);
    else if (args_ == oldNode)
        args_ = dynamic_cast<ExpressionList*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

}
}
