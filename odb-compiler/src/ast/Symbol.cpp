#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
Symbol::Symbol(const std::string& name, SourceLocation* location) :
    Node(location),
    name_(name)
{
}

// ----------------------------------------------------------------------------
const std::string& Symbol::name() const
{
    return name_;
}

// ----------------------------------------------------------------------------
std::string Symbol::toString() const
{
    return  "Symbol: \"" + name_ + "\"";
}

// ----------------------------------------------------------------------------
void Symbol::accept(Visitor* visitor)
{
    visitor->visitSymbol(this);
}
void Symbol::accept(ConstVisitor* visitor) const
{
    visitor->visitSymbol(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange Symbol::children()
{
    return {};
}

// ----------------------------------------------------------------------------
void Symbol::swapChild(const Node* oldNode, Node* newNode)
{
    assert(false);
}

// ----------------------------------------------------------------------------
Node* Symbol::duplicateImpl() const
{
    return new Symbol(name_, location());
}

}
