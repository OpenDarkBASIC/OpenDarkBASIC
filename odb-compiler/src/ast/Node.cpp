#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/ast/Visitor.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/commands/Command.hpp"
#include "odb-compiler/parsers/db/Parser.y.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
Node::Node(Program* program, SourceLocation* location) :
    program_(program),
    location_(location)
{
}

// ----------------------------------------------------------------------------
Program* Node::program() const
{
    return program_;
}

// ----------------------------------------------------------------------------
SourceLocation* Node::location() const
{
    return location_;
}

// ----------------------------------------------------------------------------
void Node::updateLocation(SourceLocation* location)
{
    location_ = location;
}


}
