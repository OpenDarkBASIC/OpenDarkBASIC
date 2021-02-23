#include "odb-compiler/ast/FuncDecl.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/ArgList.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
FuncDecl::FuncDecl(AnnotatedSymbol* symbol, ArgList* args, Block* body, Expression* returnValue, SourceLocation* location) :
    Statement(location),
    symbol_(symbol),
    args_(args),
    body_(body),
    returnValue_(returnValue)
{
    symbol->setParent(this);
    args->setParent(this);
    body->setParent(this);
    returnValue->setParent(this);
}

// ----------------------------------------------------------------------------
FuncDecl::FuncDecl(AnnotatedSymbol* symbol, ArgList* args, Expression* returnValue, SourceLocation* location) :
    Statement(location),
    symbol_(symbol),
    args_(args),
    returnValue_(returnValue)
{
    symbol->setParent(this);
    args->setParent(this);
    returnValue->setParent(this);
}

// ----------------------------------------------------------------------------
FuncDecl::FuncDecl(AnnotatedSymbol* symbol, Block* body, Expression* returnValue, SourceLocation* location) :
    Statement(location),
    symbol_(symbol),
    body_(body),
    returnValue_(returnValue)
{
    symbol->setParent(this);
    body->setParent(this);
    returnValue->setParent(this);
}

// ----------------------------------------------------------------------------
FuncDecl::FuncDecl(AnnotatedSymbol* symbol, Expression* returnValue, SourceLocation* location) :
    Statement(location),
    symbol_(symbol),
    returnValue_(returnValue)
{
    symbol->setParent(this);
    returnValue->setParent(this);
}

// ----------------------------------------------------------------------------
FuncDecl::FuncDecl(AnnotatedSymbol* symbol, ArgList* args, Block* body, SourceLocation* location) :
    Statement(location),
    symbol_(symbol),
    args_(args),
    body_(body)
{
    symbol->setParent(this);
    args->setParent(this);
    body->setParent(this);
}

// ----------------------------------------------------------------------------
FuncDecl::FuncDecl(AnnotatedSymbol* symbol, ArgList* args, SourceLocation* location) :
    Statement(location),
    symbol_(symbol),
    args_(args)
{
    symbol->setParent(this);
    args->setParent(this);
}

// ----------------------------------------------------------------------------
FuncDecl::FuncDecl(AnnotatedSymbol* symbol, Block* body, SourceLocation* location) :
    Statement(location),
    symbol_(symbol),
    body_(body)
{
    symbol->setParent(this);
    body->setParent(this);
}

// ----------------------------------------------------------------------------
FuncDecl::FuncDecl(AnnotatedSymbol* symbol, SourceLocation* location) :
    Statement(location),
    symbol_(symbol)
{
    symbol->setParent(this);
}

// ----------------------------------------------------------------------------
AnnotatedSymbol* FuncDecl::symbol() const
{
    return symbol_;
}

// ----------------------------------------------------------------------------
MaybeNull<ArgList> FuncDecl::args() const
{
    return args_.get();
}

// ----------------------------------------------------------------------------
MaybeNull<Block> FuncDecl::body() const
{
    return body_.get();
}

// ----------------------------------------------------------------------------
MaybeNull<Expression> FuncDecl::returnValue() const
{
    return returnValue_.get();
}

// ----------------------------------------------------------------------------
std::string FuncDecl::toString() const
{
    return "FuncDecl";
}

// ----------------------------------------------------------------------------
void FuncDecl::accept(Visitor* visitor)
{
    visitor->visitFuncDecl(this);
    symbol_->accept(visitor);
    if (args_)
        args_->accept(visitor);
    if (body_)
        body_->accept(visitor);
    if (returnValue_)
        returnValue_->accept(visitor);
}
void FuncDecl::accept(ConstVisitor* visitor) const
{
    visitor->visitFuncDecl(this);
    symbol_->accept(visitor);
    if (args_)
        args_->accept(visitor);
    if (body_)
        body_->accept(visitor);
    if (returnValue_)
        returnValue_->accept(visitor);
}

// ----------------------------------------------------------------------------
void FuncDecl::swapChild(const Node* oldNode, Node* newNode)
{
    if (symbol_ == oldNode)
        symbol_ = dynamic_cast<AnnotatedSymbol*>(newNode);
    else if (body_ == oldNode)
        body_ = dynamic_cast<Block*>(newNode);
    else if (returnValue_ == oldNode)
        returnValue_ = dynamic_cast<Expression*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* FuncDecl::duplicateImpl() const
{
    return new FuncDecl(
        symbol_->duplicate<AnnotatedSymbol>(),
        args_ ? args_->duplicate<ArgList>() : nullptr,
        body_ ? body_->duplicate<Block>() : nullptr,
        returnValue_ ? returnValue_->duplicate<Expression>() : nullptr,
        location());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
FuncExit::FuncExit(Expression* returnValue, SourceLocation* location) :
    Statement(location),
    returnValue_(returnValue)
{
    returnValue->setParent(this);
}

// ----------------------------------------------------------------------------
FuncExit::FuncExit(SourceLocation* location) :
    Statement(location)
{
}

// ----------------------------------------------------------------------------
MaybeNull<Expression> FuncExit::returnValue() const
{
    return returnValue_.get();
}

// ----------------------------------------------------------------------------
std::string FuncExit::toString() const
{
    return "FuncExit";
}

// ----------------------------------------------------------------------------
void FuncExit::accept(Visitor* visitor)
{
    visitor->visitFuncExit(this);
    if (returnValue_)
        returnValue_->accept(visitor);
}
void FuncExit::accept(ConstVisitor* visitor) const
{
    visitor->visitFuncExit(this);
    if (returnValue_)
        returnValue_->accept(visitor);
}

// ----------------------------------------------------------------------------
void FuncExit::swapChild(const Node* oldNode, Node* newNode)
{
    if (returnValue_ == oldNode)
        returnValue_ = dynamic_cast<Expression*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* FuncExit::duplicateImpl() const
{
    return new FuncExit(
        returnValue_ ? returnValue_->duplicate<Expression>() : nullptr,
        location());
}

}
}
