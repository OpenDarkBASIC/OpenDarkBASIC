#include <utility>

#include "odb-compiler/ast/CommandExpr.hpp"
#include "odb-compiler/ast/ArgList.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"
#include "odb-compiler/commands/Command.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
CommandExpr::CommandExpr(std::string commandName, ArgList* args, SourceLocation* location) :
    Expression(location),
    commandName_(std::move(commandName)),
    args_(args),
    command_(nullptr)
{
}

// ----------------------------------------------------------------------------
CommandExpr::CommandExpr(std::string commandName, SourceLocation* location) :
    Expression(location),
    commandName_(std::move(commandName)),
    command_(nullptr)
{
}

// ----------------------------------------------------------------------------
const std::string& CommandExpr::commandName() const
{
    return commandName_;
}

// ----------------------------------------------------------------------------
MaybeNull<ArgList> CommandExpr::args() const
{
    return args_.get();
}

// ----------------------------------------------------------------------------
const cmd::Command* CommandExpr::command() const
{
    assert(command_ && command_->dbSymbol() == commandName_);
    return command_;
}

// ----------------------------------------------------------------------------
void CommandExpr::setCommand(const cmd::Command* command)
{
    command_ = command;
}

// ----------------------------------------------------------------------------
Type CommandExpr::getType() const
{
    if (!command_)
    {
        return Type::getUnknown();
    }
    return Type::getFromCommandType(command_->returnType());
}

// ----------------------------------------------------------------------------
std::string CommandExpr::toString() const
{
    return "CommandExpr: \"" + commandName_ + "\"";
}

// ----------------------------------------------------------------------------
void CommandExpr::accept(Visitor* visitor)
{
    visitor->visitCommandExpr(this);
}
void CommandExpr::accept(ConstVisitor* visitor) const
{
    visitor->visitCommandExpr(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange CommandExpr::children()
{
    if (args_)
    {
        return {args_};
    }
    else
    {
        return {};
    }
}

// ----------------------------------------------------------------------------
void CommandExpr::swapChild(const Node* oldNode, Node* newNode)
{
    if (args_ == oldNode)
        args_ = dynamic_cast<ArgList*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* CommandExpr::duplicateImpl() const
{
    auto* newCommand = new CommandExpr(
        commandName_,
        args_ ? args_->duplicate<ArgList>() : nullptr,
        location());
    newCommand->command_ = command_;
    return newCommand;
}

}

