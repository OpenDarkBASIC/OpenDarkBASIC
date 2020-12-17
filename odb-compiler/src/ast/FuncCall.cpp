#include "odb-compiler/ast/FuncCall.hpp"
#include "odb-compiler/ast/AnnotatedSymbol.hpp"
#include "odb-compiler/ast/ExpressionList.hpp"
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
ExpressionList* FuncCallExpr::args() const
{
    return args_;
}

// ----------------------------------------------------------------------------
void FuncCallExpr::accept(Visitor* visitor) const
{
    visitor->visitFuncCallExpr(this);
    symbol_->accept(visitor);
    if (args_)
        args_->accept(visitor);
}

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
ExpressionList* FuncCallExprOrArrayRef::args() const
{
    return args_;
}

// ----------------------------------------------------------------------------
void FuncCallExprOrArrayRef::accept(Visitor* visitor) const
{
    visitor->visitFuncCallExprOrArrayRef(this);
    symbol_->accept(visitor);
    args_->accept(visitor);
}

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
ExpressionList* FuncCallStmnt::args() const
{
    return args_;
}

// ----------------------------------------------------------------------------
void FuncCallStmnt::accept(Visitor* visitor) const
{
    visitor->visitFuncCallStmnt(this);
    symbol_->accept(visitor);
    if (args_)
        args_->accept(visitor);
}

}
}
