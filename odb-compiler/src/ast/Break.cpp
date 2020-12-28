#include "odb-compiler/ast/Break.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
Break::Break(SourceLocation* location) :
    Statement(location)
{
}

// ----------------------------------------------------------------------------
void Break::accept(Visitor* visitor)
{
    visitor->visitBreak(this);
}
void Break::accept(ConstVisitor* visitor) const
{
    visitor->visitBreak(this);
}

// ----------------------------------------------------------------------------
void Break::swapChild(const Node* oldNode, Node* newNode)
{
    assert(false);
}

}
}
