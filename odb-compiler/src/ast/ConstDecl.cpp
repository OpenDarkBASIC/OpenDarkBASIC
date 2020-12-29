#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/ConstDecl.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
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
void ConstDecl::accept(Visitor* visitor)
{
    visitor->visitConstDecl(this);
    symbol_->accept(visitor);
    literal_->accept(visitor);
}
void ConstDecl::accept(ConstVisitor* visitor) const
{
    visitor->visitConstDecl(this);
    symbol_->accept(visitor);
    literal_->accept(visitor);
}

// ----------------------------------------------------------------------------
void ConstDecl::swapChild(const Node* oldNode, Node* newNode)
{
    if (symbol_ == oldNode)
        symbol_ = dynamic_cast<AnnotatedSymbol*>(newNode);
    else if (literal_ == oldNode)
        literal_ = dynamic_cast<Literal*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

}
}
