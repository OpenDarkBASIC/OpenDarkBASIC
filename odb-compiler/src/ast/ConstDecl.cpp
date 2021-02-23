#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/ConstDecl.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
ConstDeclExpr::ConstDeclExpr(AnnotatedSymbol* symbol, Expression* expr, SourceLocation* location) :
    Statement(location),
    symbol_(symbol),
    expr_(expr)
{
    symbol->setParent(this);
    expr->setParent(this);
}

// ----------------------------------------------------------------------------
AnnotatedSymbol* ConstDeclExpr::symbol() const
{
    return symbol_;
}

// ----------------------------------------------------------------------------
Expression* ConstDeclExpr::expression() const
{
    return expr_;
}

// ----------------------------------------------------------------------------
std::string ConstDeclExpr::toString() const
{
    return "ConstDeclExpr";
}

// ----------------------------------------------------------------------------
void ConstDeclExpr::accept(Visitor* visitor)
{
    visitor->visitConstDeclExpr(this);
    symbol_->accept(visitor);
    expr_->accept(visitor);
}
void ConstDeclExpr::accept(ConstVisitor* visitor) const
{
    visitor->visitConstDeclExpr(this);
    symbol_->accept(visitor);
    expr_->accept(visitor);
}

// ----------------------------------------------------------------------------
void ConstDeclExpr::swapChild(const Node* oldNode, Node* newNode)
{
    if (symbol_ == oldNode)
        symbol_ = dynamic_cast<AnnotatedSymbol*>(newNode);
    else if (expr_ == oldNode)
        expr_ = dynamic_cast<Expression*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* ConstDeclExpr::duplicateImpl() const
{
    return new ConstDeclExpr(
        symbol_->duplicate<AnnotatedSymbol>(),
        expr_->duplicate<Expression>(),
        location());
}

// ============================================================================
// ============================================================================

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
std::string ConstDecl::toString() const
{
    return "ConstDecl";
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

// ----------------------------------------------------------------------------
Node* ConstDecl::duplicateImpl() const
{
    return new ConstDecl(
        symbol_->duplicate<AnnotatedSymbol>(),
        literal_->duplicate<Literal>(),
        location());
}

}
