#include "odb-compiler/ast/FuncCall.hpp"
#include "odb-compiler/ast/AnnotatedSymbol.hpp"
#include "odb-compiler/ast/ArgList.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
FuncCallExpr::FuncCallExpr(AnnotatedSymbol* symbol, ArgList* args, SourceLocation* location) :
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
MaybeNull<ArgList> FuncCallExpr::args() const
{
    return args_.get();
}

// ----------------------------------------------------------------------------
std::string FuncCallExpr::toString() const
{
    return "FuncCallExpr";
}

// ----------------------------------------------------------------------------
void FuncCallExpr::accept(Visitor* visitor)
{
    visitor->visitFuncCallExpr(this);
}
void FuncCallExpr::accept(ConstVisitor* visitor) const
{
    visitor->visitFuncCallExpr(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange FuncCallExpr::children()
{
    if (args_)
    {
        return {symbol_, args_};
    }
    else
    {
        return {symbol_};
    }
}

// ----------------------------------------------------------------------------
void FuncCallExpr::swapChild(const Node* oldNode, Node* newNode)
{
    if (symbol_ == oldNode)
        symbol_ = dynamic_cast<AnnotatedSymbol*>(newNode);
    else if (args_ == oldNode)
        args_ = dynamic_cast<ArgList*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* FuncCallExpr::duplicateImpl() const
{
    return new FuncCallExpr(
        symbol_->duplicate<AnnotatedSymbol>(),
        args_ ? args_->duplicate<ArgList>() : nullptr,
        location());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
FuncCallExprOrArrayRef::FuncCallExprOrArrayRef(AnnotatedSymbol* symbol, ArgList* args, SourceLocation* location) :
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
MaybeNull<ArgList> FuncCallExprOrArrayRef::args() const
{
    return args_.get();
}

// ----------------------------------------------------------------------------
std::string FuncCallExprOrArrayRef::toString() const
{
    return "FuncCallExprOrArrayRef";
}

// ----------------------------------------------------------------------------
void FuncCallExprOrArrayRef::accept(Visitor* visitor)
{
    visitor->visitFuncCallExprOrArrayRef(this);
}
void FuncCallExprOrArrayRef::accept(ConstVisitor* visitor) const
{
    visitor->visitFuncCallExprOrArrayRef(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange FuncCallExprOrArrayRef::children()
{
    if (args_)
    {
        return {symbol_, args_};
    }
    else
    {
        return {symbol_};
    }
}

// ----------------------------------------------------------------------------
void FuncCallExprOrArrayRef::swapChild(const Node* oldNode, Node* newNode)
{
    if (symbol_ == oldNode)
        symbol_ = dynamic_cast<AnnotatedSymbol*>(newNode);
    else if (args_ == oldNode)
        args_ = dynamic_cast<ArgList*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* FuncCallExprOrArrayRef::duplicateImpl() const
{
    return new FuncCallExprOrArrayRef(
        symbol_->duplicate<AnnotatedSymbol>(),
        args_ ? args_->duplicate<ArgList>() : nullptr,
        location());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
FuncCallStmnt::FuncCallStmnt(AnnotatedSymbol* symbol, ArgList* args, SourceLocation* location) :
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
MaybeNull<ArgList> FuncCallStmnt::args() const
{
    return args_.get();
}

// ----------------------------------------------------------------------------
std::string FuncCallStmnt::toString() const
{
    return "FuncCallStmnt";
}

// ----------------------------------------------------------------------------
void FuncCallStmnt::accept(Visitor* visitor)
{
    visitor->visitFuncCallStmnt(this);
}
void FuncCallStmnt::accept(ConstVisitor* visitor) const
{
    visitor->visitFuncCallStmnt(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange FuncCallStmnt::children()
{
    if (args_)
    {
        return {symbol_, args_};
    }
    else
    {
        return {symbol_};
    }
}

// ----------------------------------------------------------------------------
void FuncCallStmnt::swapChild(const Node* oldNode, Node* newNode)
{
    if (symbol_ == oldNode)
        symbol_ = dynamic_cast<AnnotatedSymbol*>(newNode);
    else if (args_ == oldNode)
        args_ = dynamic_cast<ArgList*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* FuncCallStmnt::duplicateImpl() const
{
    return new FuncCallStmnt(
        symbol_->duplicate<AnnotatedSymbol>(),
        args_ ? args_->duplicate<ArgList>() : nullptr,
        location());
}

}
