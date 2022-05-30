#include "odb-compiler/ast/FuncCall.hpp"
#include "odb-compiler/ast/ArgList.hpp"
#include "odb-compiler/ast/Identifier.hpp"
#include "odb-compiler/ast/FuncDecl.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
FuncCallExpr::FuncCallExpr(Program* program, SourceLocation* location, Identifier* identifier, ArgList* args) :
    Expression(program, location),
    identifier_(identifier),
    args_(args),
    function_(nullptr)
{
}

// ----------------------------------------------------------------------------
FuncCallExpr::FuncCallExpr(Program* program, SourceLocation* location, Identifier* identifier) :
    Expression(program, location),
    identifier_(identifier),
    function_(nullptr)
{
}

// ----------------------------------------------------------------------------
Identifier* FuncCallExpr::identifier() const
{
    return identifier_;
}

// ----------------------------------------------------------------------------
MaybeNull<ArgList> FuncCallExpr::args() const
{
    return args_.get();
}

// ----------------------------------------------------------------------------
FuncDecl* FuncCallExpr::function() const
{
    assert(function_ && function_->identifier()->name() == identifier_->name());
    return function_;
}

// ----------------------------------------------------------------------------
void FuncCallExpr::setFunction(FuncDecl* func)
{
    assert(func->identifier()->name() == identifier_->name());
    function_ = func;
}

// ----------------------------------------------------------------------------
Type FuncCallExpr::getType() const
{
    if (!function_)
    {
        return Type::getUnknown();
    }
    return function_->returnValue() ? function_->returnValue()->getType() : Type::getVoid();
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
        return {identifier_, args_};
    }
    else
    {
        return {identifier_};
    }
}

// ----------------------------------------------------------------------------
void FuncCallExpr::swapChild(const Node* oldNode, Node* newNode)
{
    if (identifier_ == oldNode)
        identifier_ = dynamic_cast<Identifier*>(newNode);
    else if (args_ == oldNode)
        args_ = dynamic_cast<ArgList*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* FuncCallExpr::duplicateImpl() const
{
    return new FuncCallExpr(
        program(),
        location(),
        identifier_->duplicate<Identifier>(),
        args_ ? args_->duplicate<ArgList>() : nullptr);
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
FuncCallStmnt::FuncCallStmnt(Program* program, SourceLocation* location, Identifier* identifier, ArgList* args)
    : Statement(program, location),
      identifier_(identifier),
      args_(args),
      function_(nullptr)
{
}

// ----------------------------------------------------------------------------
FuncCallStmnt::FuncCallStmnt(Program* program, SourceLocation* location, Identifier* symbol)
    : Statement(program, location),
      identifier_(symbol),
      function_(nullptr)
{
}

// ----------------------------------------------------------------------------
Identifier* FuncCallStmnt::identifier() const
{
    return identifier_;
}

// ----------------------------------------------------------------------------
MaybeNull<ArgList> FuncCallStmnt::args() const
{
    return args_.get();
}

// ----------------------------------------------------------------------------
FuncDecl* FuncCallStmnt::function() const
{
    assert(function_ && function_->identifier()->name() == identifier_->name());
    return function_;
}

// ----------------------------------------------------------------------------
void FuncCallStmnt::setFunction(FuncDecl* func)
{
    assert(func->identifier()->name() == identifier_->name());
    function_ = func;
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
        return {identifier_, args_};
    }
    else
    {
        return {identifier_};
    }
}

// ----------------------------------------------------------------------------
void FuncCallStmnt::swapChild(const Node* oldNode, Node* newNode)
{
    if (identifier_ == oldNode)
        identifier_ = dynamic_cast<Identifier*>(newNode);
    else if (args_ == oldNode)
        args_ = dynamic_cast<ArgList*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* FuncCallStmnt::duplicateImpl() const
{
    return new FuncCallStmnt(
        program(),
        location(),
        identifier_->duplicate<Identifier>(),
        args_ ? args_->duplicate<ArgList>() : nullptr);
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
FuncCallExprOrArrayRef::FuncCallExprOrArrayRef(Program* program, SourceLocation* location, Identifier* identifier, ArgList* args) :
    Expression(program, location),
    identifier_(identifier),
    args_(args)
{
}

// ----------------------------------------------------------------------------
FuncCallExprOrArrayRef::FuncCallExprOrArrayRef(Program* program, SourceLocation* location, Identifier* identifier) :
    Expression(program, location),
    identifier_(identifier),
    args_(nullptr)
{
}

// ----------------------------------------------------------------------------
Identifier* FuncCallExprOrArrayRef::identifier() const
{
    return identifier_;
}

// ----------------------------------------------------------------------------
MaybeNull<ArgList> FuncCallExprOrArrayRef::args() const
{
    return args_.get();
}

// ----------------------------------------------------------------------------
Type FuncCallExprOrArrayRef::getType() const
{
    return Type::getUnknown();
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
        return {identifier_, args_};
    }
    else
    {
        return {identifier_};
    }
}

// ----------------------------------------------------------------------------
void FuncCallExprOrArrayRef::swapChild(const Node* oldNode, Node* newNode)
{
    if (identifier_ == oldNode)
        identifier_ = dynamic_cast<Identifier*>(newNode);
    else if (args_ == oldNode)
        args_ = dynamic_cast<ArgList*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* FuncCallExprOrArrayRef::duplicateImpl() const
{
    return new FuncCallExprOrArrayRef(
        program(),
        location(),
        identifier_->duplicate<Identifier>(),
        args_ ? args_->duplicate<ArgList>() : nullptr);
}

}
