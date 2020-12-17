#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb {
namespace ast {

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
void Symbol::accept(Visitor* visitor) const
{
    visitor->visitSymbol(this);
}

}
}
