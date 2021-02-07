#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/UDTRef.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
UDTRef::UDTRef(const std::string& name, SourceLocation* location)
    : Symbol(name, location)
{
}

// ----------------------------------------------------------------------------
void UDTRef::accept(Visitor* visitor)
{
    visitor->visitUDTRef(this);
}
void UDTRef::accept(ConstVisitor* visitor) const
{
    visitor->visitUDTRef(this);
}

// ----------------------------------------------------------------------------
Node* UDTRef::duplicateImpl() const
{
    return new UDTRef(name_, location());
}

}
