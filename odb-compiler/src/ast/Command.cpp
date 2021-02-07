#include "odb-compiler/ast/Command.hpp"
#include "odb-compiler/ast/ExpressionList.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"
#include "odb-compiler/commands/Command.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
CommandExprSymbol::CommandExprSymbol(const std::string& command, ExpressionList* args, SourceLocation* location) :
    Expression(location),
    args_(args),
    command_(command)
{
    args->setParent(this);
}
CommandExprSymbol::CommandExprSymbol(const std::string& command, SourceLocation* location) :
    Expression(location),
    command_(command)
{
}
const std::string& CommandExprSymbol::command() const
{
    return command_;
}
MaybeNull<ExpressionList> CommandExprSymbol::args() const
{
    return args_.get();
}
void CommandExprSymbol::accept(Visitor* visitor)
{
    visitor->visitCommandExprSymbol(this);
    if (args_)
        args_->accept(visitor);
}
void CommandExprSymbol::accept(ConstVisitor* visitor) const
{
    visitor->visitCommandExprSymbol(this);
    if (args_)
        args_->accept(visitor);
}

// ----------------------------------------------------------------------------
void CommandExprSymbol::swapChild(const Node* oldNode, Node* newNode)
{
    if (args_ == oldNode)
        args_ = dynamic_cast<ExpressionList*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* CommandExprSymbol::duplicateImpl() const
{
    return new CommandExprSymbol(
        command_,
        args_ ? args_->duplicate<ExpressionList>() : nullptr,
        location());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
CommandStmntSymbol::CommandStmntSymbol(const std::string& command, ExpressionList* args, SourceLocation* location) :
    Statement(location),
    args_(args),
    command_(command)
{
    args->setParent(this);
}
CommandStmntSymbol::CommandStmntSymbol(const std::string& command, SourceLocation* location) :
    Statement(location),
    command_(command)
{
}
const std::string& CommandStmntSymbol::command() const
{
    return command_;
}
MaybeNull<ExpressionList> CommandStmntSymbol::args() const
{
    return args_.get();
}
void CommandStmntSymbol::accept(Visitor* visitor)
{
    visitor->visitCommandStmntSymbol(this);
    if (args_)
        args_->accept(visitor);
}
void CommandStmntSymbol::accept(ConstVisitor* visitor) const
{
    visitor->visitCommandStmntSymbol(this);
    if (args_)
        args_->accept(visitor);
}

// ----------------------------------------------------------------------------
void CommandStmntSymbol::swapChild(const Node* oldNode, Node* newNode)
{
    if (args_ == oldNode)
        args_ = dynamic_cast<ExpressionList*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* CommandStmntSymbol::duplicateImpl() const
{
    return new CommandStmntSymbol(
        command_,
        args_ ? args_->duplicate<ExpressionList>() : nullptr,
        location());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
CommandExpr::CommandExpr(cmd::Command* command, ExpressionList* args, SourceLocation* location) :
    Expression(location),
    command_(command),
    args_(args)
{
    args->setParent(this);
}
CommandExpr::CommandExpr(cmd::Command* command, SourceLocation* location) :
    Expression(location),
    command_(command)
{
}
cmd::Command* CommandExpr::command() const
{
    return command_;
}
MaybeNull<ExpressionList> CommandExpr::args() const
{
    return args_.get();
}
void CommandExpr::accept(Visitor* visitor)
{
    visitor->visitCommandExpr(this);
    if (args_)
        args_->accept(visitor);
}
void CommandExpr::accept(ConstVisitor* visitor) const
{
    visitor->visitCommandExpr(this);
    if (args_)
        args_->accept(visitor);
}

// ----------------------------------------------------------------------------
void CommandExpr::swapChild(const Node* oldNode, Node* newNode)
{
    if (args_ == oldNode)
        args_ = dynamic_cast<ExpressionList*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* CommandExpr::duplicateImpl() const
{
    return new CommandExpr(
        command_,
        args_ ? args_->duplicate<ExpressionList>() : nullptr,
        location());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
CommandStmnt::CommandStmnt(cmd::Command* command, ExpressionList* args, SourceLocation* location) :
    Statement(location),
    command_(command),
    args_(args)
{
    args->setParent(this);
}
CommandStmnt::CommandStmnt(cmd::Command* command, SourceLocation* location) :
    Statement(location),
    command_(command)
{
}
cmd::Command* CommandStmnt::command() const
{
    return command_;
}
MaybeNull<ExpressionList> CommandStmnt::args() const
{
    return args_.get();
}
void CommandStmnt::accept(Visitor* visitor)
{
    visitor->visitCommandStmnt(this);
    if (args_)
        args_->accept(visitor);
}
void CommandStmnt::accept(ConstVisitor* visitor) const
{
    visitor->visitCommandStmnt(this);
    if (args_)
        args_->accept(visitor);
}

// ----------------------------------------------------------------------------
void CommandStmnt::swapChild(const Node* oldNode, Node* newNode)
{
    if (args_ == oldNode)
        args_ = dynamic_cast<ExpressionList*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* CommandStmnt::duplicateImpl() const
{
    return new CommandStmnt(
        command_,
        args_ ? args_->duplicate<ExpressionList>() : nullptr,
        location());
}

}
