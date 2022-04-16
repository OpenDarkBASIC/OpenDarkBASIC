#include "odb-compiler/ast/CommandExpr.hpp"
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

}

