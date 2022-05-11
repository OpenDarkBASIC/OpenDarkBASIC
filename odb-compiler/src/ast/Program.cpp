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
VariableScope& Program::mainScope()
{
    return mainScope_;
}

// ----------------------------------------------------------------------------
const VariableScope& Program::mainScope() const
{
    return mainScope_;
}

// ----------------------------------------------------------------------------
VariableScope& Program::globalScope()
{
    return globalScope_;
}

// ----------------------------------------------------------------------------
const VariableScope& Program::globalScope() const
{
    return globalScope_;
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
    Program* program = new Program(body_->duplicate<Block>(), location());
    program->mainScope_ = mainScope_;
    program->globalScope_ = globalScope_;
    return program;
}

}
