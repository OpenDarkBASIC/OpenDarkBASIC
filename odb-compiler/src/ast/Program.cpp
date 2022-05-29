#include "odb-compiler/ast/Program.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/UDTDecl.hpp"
#include "odb-compiler/ast/Identifier.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
Program::Program() :
    Node(this, nullptr),
    body_(nullptr)
{
}

// ----------------------------------------------------------------------------
void Program::setBody(Block* body)
{
    body_ = body;
    updateLocation(body->location());
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
void Program::addUDT(UDTDecl* decl)
{
    udts_[decl->typeName()->name()] = decl;
}

// ----------------------------------------------------------------------------
UDTDecl* Program::lookupUDT(const std::string& name) const
{
    return udts_.count(name) > 0 ? udts_.at(name) : nullptr;
}

// ----------------------------------------------------------------------------
std::vector<UDTDecl*> Program::getUDTList() const
{
    std::vector<UDTDecl*> out;
    out.reserve(udts_.size());
    for (auto [_, v] : udts_) {
        out.emplace_back(v);
    }
    return out;
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
    Program* program = new Program();
    program->updateLocation(location());
    program->body_ = body_->duplicate<Block>();
    return program;
}

}
