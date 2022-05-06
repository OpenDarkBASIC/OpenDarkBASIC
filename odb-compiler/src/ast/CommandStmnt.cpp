#include <utility>

#include "odb-compiler/ast/CommandStmnt.hpp"
#include "odb-compiler/ast/ArgList.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"
#include "odb-compiler/commands/Command.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
CommandStmnt::CommandStmnt(std::string commandName, ArgList* args, SourceLocation* location) :
    Statement(location),
    commandName_(std::move(commandName)),
    args_(args),
    command_(nullptr)
{
}

// ----------------------------------------------------------------------------
CommandStmnt::CommandStmnt(std::string commandName, SourceLocation* location) :
    Statement(location),
    commandName_(std::move(commandName)),
    command_(nullptr)
{
}

// ----------------------------------------------------------------------------
const std::string& CommandStmnt::commandName() const
{
    return commandName_;
}

// ----------------------------------------------------------------------------
MaybeNull<ArgList> CommandStmnt::args() const
{
    return args_.get();
}

// ----------------------------------------------------------------------------
const cmd::Command* CommandStmnt::command() const
{
    assert(command_ && command_->dbSymbol() == commandName_);
    return command_;
}

// ----------------------------------------------------------------------------
void CommandStmnt::setCommand(const cmd::Command* command)
{
    command_ = command;
}

// ----------------------------------------------------------------------------
std::string CommandStmnt::toString() const
{
    return "CommandStmnt: \"" + commandName_ + "\"";
}

// ----------------------------------------------------------------------------
void CommandStmnt::accept(Visitor* visitor)
{
    visitor->visitCommandStmnt(this);
}
void CommandStmnt::accept(ConstVisitor* visitor) const
{
    visitor->visitCommandStmnt(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange CommandStmnt::children()
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
void CommandStmnt::swapChild(const Node* oldNode, Node* newNode)
{
    if (args_ == oldNode)
        args_ = dynamic_cast<ArgList*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* CommandStmnt::duplicateImpl() const
{
    auto* newCommand = new CommandStmnt(
        commandName_,
        args_ ? args_->duplicate<ArgList>() : nullptr,
        location());
    newCommand->command_ = command_;
    return newCommand;
}

}
