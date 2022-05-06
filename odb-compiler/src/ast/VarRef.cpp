#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/ast/Identifier.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Variable.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
VarRef::VarRef(Identifier* identifier, SourceLocation* location) :
    LValue(location),
    identifier_(identifier),
    variable_(nullptr)
{
}

// ----------------------------------------------------------------------------
Identifier* VarRef::identifier() const
{
    return identifier_;
}

// ----------------------------------------------------------------------------
Type VarRef::getType() const
{
    return variable_ ? variable_->getType() : Type::getUnknown();
}

// ----------------------------------------------------------------------------
void VarRef::setVariable(Variable* variable)
{
    assert(identifier_->name() == variable->name());
    variable_ = variable;
}

// ----------------------------------------------------------------------------
Variable* VarRef::variable() const
{
    assert(identifier_->name() == variable_->name());
    return variable_;
}

// ----------------------------------------------------------------------------
std::string VarRef::toString() const
{
    return "VarRef";
}

// ----------------------------------------------------------------------------
void VarRef::accept(Visitor* visitor)
{
    visitor->visitVarRef(this);
}
void VarRef::accept(ConstVisitor* visitor) const
{
    visitor->visitVarRef(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange VarRef::children()
{
    return {identifier_};
}

// ----------------------------------------------------------------------------
void VarRef::swapChild(const Node* oldNode, Node* newNode)
{
    if (identifier_ == oldNode)
        identifier_ = dynamic_cast<Identifier*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* VarRef::duplicateImpl() const
{
    return new VarRef(
        identifier_->duplicate<Identifier>(),
        location());
}

}
