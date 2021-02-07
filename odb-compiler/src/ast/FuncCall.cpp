#include "odb-compiler/ast/FuncCall.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/ExpressionList.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
FuncCallExpr::FuncCallExpr(AnnotatedSymbol* symbol, ExpressionList* args, SourceLocation* location) :
    Expression(location),
    symbol_(symbol),
    args_(args)
{
    symbol->setParent(this);
    args->setParent(this);
}

// ----------------------------------------------------------------------------
FuncCallExpr::FuncCallExpr(AnnotatedSymbol* symbol, SourceLocation* location) :
    Expression(location),
    symbol_(symbol)
{
    symbol->setParent(this);
}

// ----------------------------------------------------------------------------
AnnotatedSymbol* FuncCallExpr::symbol() const
{
    return symbol_;
}

// ----------------------------------------------------------------------------
MaybeNull<ExpressionList> FuncCallExpr::args() const
{
    return args_.get();
}

// ----------------------------------------------------------------------------
void FuncCallExpr::accept(Visitor* visitor)
{
    visitor->visitFuncCallExpr(this);
    symbol_->accept(visitor);
    if (args_)
        args_->accept(visitor);
}
void FuncCallExpr::accept(ConstVisitor* visitor) const
{
    visitor->visitFuncCallExpr(this);
    symbol_->accept(visitor);
    if (args_)
        args_->accept(visitor);
}

// ----------------------------------------------------------------------------
void FuncCallExpr::swapChild(const Node* oldNode, Node* newNode)
{
    if (symbol_ == oldNode)
        symbol_ = dynamic_cast<AnnotatedSymbol*>(newNode);
    else if (args_ == oldNode)
        args_ = dynamic_cast<ExpressionList*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* FuncCallExpr::duplicateImpl() const
{
    return new FuncCallExpr(
        symbol_->duplicate<AnnotatedSymbol>(),
        args_ ? args_->duplicate<ExpressionList>() : nullptr,
        location());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
FuncCallExprOrArrayRef::FuncCallExprOrArrayRef(AnnotatedSymbol* symbol, ExpressionList* args, SourceLocation* location) :
    Expression(location),
    symbol_(symbol),
    args_(args)
{
    symbol->setParent(this);
    args->setParent(this);
}

// ----------------------------------------------------------------------------
AnnotatedSymbol* FuncCallExprOrArrayRef::symbol() const
{
    return symbol_;
}

// ----------------------------------------------------------------------------
MaybeNull<ExpressionList> FuncCallExprOrArrayRef::args() const
{
    return args_.get();
}

// ----------------------------------------------------------------------------
void FuncCallExprOrArrayRef::accept(Visitor* visitor)
{
    visitor->visitFuncCallExprOrArrayRef(this);
    symbol_->accept(visitor);
    if (args_)
        args_->accept(visitor);
}
void FuncCallExprOrArrayRef::accept(ConstVisitor* visitor) const
{
    visitor->visitFuncCallExprOrArrayRef(this);
    symbol_->accept(visitor);
    if (args_)
        args_->accept(visitor);
}

// ----------------------------------------------------------------------------
void FuncCallExprOrArrayRef::swapChild(const Node* oldNode, Node* newNode)
{
    if (symbol_ == oldNode)
        symbol_ = dynamic_cast<AnnotatedSymbol*>(newNode);
    else if (args_ == oldNode)
        args_ = dynamic_cast<ExpressionList*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* FuncCallExprOrArrayRef::duplicateImpl() const
{
    return new FuncCallExprOrArrayRef(
        symbol_->duplicate<AnnotatedSymbol>(),
        args_ ? args_->duplicate<ExpressionList>() : nullptr,
        location());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
FuncCallStmnt::FuncCallStmnt(AnnotatedSymbol* symbol, ExpressionList* args, SourceLocation* location) :
    Statement(location),
    symbol_(symbol),
    args_(args)
{
    symbol->setParent(this);
    args->setParent(this);
}

// ----------------------------------------------------------------------------
FuncCallStmnt::FuncCallStmnt(AnnotatedSymbol* symbol, SourceLocation* location) :
    Statement(location),
    symbol_(symbol)
{
    symbol->setParent(this);
}

// ----------------------------------------------------------------------------
AnnotatedSymbol* FuncCallStmnt::symbol() const
{
    return symbol_;
}

// ----------------------------------------------------------------------------
MaybeNull<ExpressionList> FuncCallStmnt::args() const
{
    return args_.get();
}

// ----------------------------------------------------------------------------
void FuncCallStmnt::accept(Visitor* visitor)
{
    visitor->visitFuncCallStmnt(this);
    symbol_->accept(visitor);
    if (args_)
        args_->accept(visitor);
}
void FuncCallStmnt::accept(ConstVisitor* visitor) const
{
    visitor->visitFuncCallStmnt(this);
    symbol_->accept(visitor);
    if (args_)
        args_->accept(visitor);
}

// ----------------------------------------------------------------------------
void FuncCallStmnt::swapChild(const Node* oldNode, Node* newNode)
{
    if (symbol_ == oldNode)
        symbol_ = dynamic_cast<AnnotatedSymbol*>(newNode);
    else if (args_ == oldNode)
        args_ = dynamic_cast<ExpressionList*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* FuncCallStmnt::duplicateImpl() const
{
    return new FuncCallStmnt(
        symbol_->duplicate<AnnotatedSymbol>(),
        args_ ? args_->duplicate<ExpressionList>() : nullptr,
        location());
}

}
}
