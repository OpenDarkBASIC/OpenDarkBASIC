#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/ast/Visitor.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/keywords/Keyword.hpp"
#include "odb-compiler/parsers/db/Parser.y.h"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
Node::Node(SourceLocation* location) :
    location_(location)
{
}

// ----------------------------------------------------------------------------
Node* Node::parent() const
{
    return parent_;
}

// ----------------------------------------------------------------------------
void Node::setParent(Node* node)
{
    parent_ = node;
}


}
}
