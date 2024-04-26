#include "odb-compiler/ast/Exit.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
Exit::Exit(Program* program, SourceLocation* location) :
    Statement(program, location)
{
}

// ----------------------------------------------------------------------------
std::string Exit::toString() const
{
    return "Exit";
}

// ----------------------------------------------------------------------------
void Exit::accept(Visitor* visitor)
{
    visitor->visitExit(this);
}
void Exit::accept(ConstVisitor* visitor) const
{
    visitor->visitExit(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange Exit::children()
{
    return {};
}

// ----------------------------------------------------------------------------
void Exit::swapChild(const Node* oldNode, Node* newNode)
{
    assert(false);
}

// ----------------------------------------------------------------------------
Node* Exit::duplicateImpl() const
{
    return new Exit(program(), location());
}

}
