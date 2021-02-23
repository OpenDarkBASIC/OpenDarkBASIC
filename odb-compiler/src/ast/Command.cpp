#include "odb-compiler/ast/Command.hpp"
#include "odb-compiler/ast/ArgList.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"
#include "odb-compiler/commands/Command.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
CommandExpr::CommandExpr(const std::string& command, ArgList* args, SourceLocation* location) :
    Expression(location),
    args_(args),
    command_(command)
{
    args->setParent(this);
}

// ----------------------------------------------------------------------------
CommandExpr::CommandExpr(const std::string& command, SourceLocation* location) :
    Expression(location),
    command_(command)
{
}

// ----------------------------------------------------------------------------
const std::string& CommandExpr::command() const
{
    return command_;
}

// ----------------------------------------------------------------------------
MaybeNull<ArgList> CommandExpr::args() const
{
    return args_.get();
}

// ----------------------------------------------------------------------------
std::string CommandExpr::toString() const
{
    return "CommandExpr: \"" + command_ + "\"";
}

// ----------------------------------------------------------------------------
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
        args_ = dynamic_cast<ArgList*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* CommandExpr::duplicateImpl() const
{
    return new CommandExpr(
        command_,
        args_ ? args_->duplicate<ArgList>() : nullptr,
        location());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
CommandStmnt::CommandStmnt(const std::string& command, ArgList* args, SourceLocation* location) :
    Statement(location),
    args_(args),
    command_(command)
{
    args->setParent(this);
}

// ----------------------------------------------------------------------------
CommandStmnt::CommandStmnt(const std::string& command, SourceLocation* location) :
    Statement(location),
    command_(command)
{
}

// ----------------------------------------------------------------------------
const std::string& CommandStmnt::command() const
{
    return command_;
}

// ----------------------------------------------------------------------------
MaybeNull<ArgList> CommandStmnt::args() const
{
    return args_.get();
}

// ----------------------------------------------------------------------------
std::string CommandStmnt::toString() const
{
    return "CommandStmnt: \"" + command_ + "\"";
}

// ----------------------------------------------------------------------------
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
        args_ = dynamic_cast<ArgList*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* CommandStmnt::duplicateImpl() const
{
    return new CommandStmnt(
        command_,
        args_ ? args_->duplicate<ArgList>() : nullptr,
        location());
}

}
