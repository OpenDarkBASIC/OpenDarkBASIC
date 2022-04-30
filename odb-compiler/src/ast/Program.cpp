#include "odb-compiler/ast/Program.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
Program::Program(Block* body, SourceLocation* location) :
    Node(location),
    body_(body)
{
}

// ----------------------------------------------------------------------------
Block* Program::body() const
{
    return body_.get();
}

// ----------------------------------------------------------------------------
std::string Program::toString() const
{
    return "Program";
}

// ----------------------------------------------------------------------------
void Program::accept(Visitor* visitor)
{
    visitor->visitProgram(this);
}
void Program::accept(ConstVisitor* visitor) const
{
    visitor->visitProgram(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange Program::children()
{
    return {body_.get()};
}

// ----------------------------------------------------------------------------
void Program::swapChild(const Node* oldNode, Node* newNode)
{
    if (body_ == oldNode)
        body_ = dynamic_cast<Block*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* Program::duplicateImpl() const
{
    return new Program(
        body_->duplicate<Block>(),
        location());
}

}
