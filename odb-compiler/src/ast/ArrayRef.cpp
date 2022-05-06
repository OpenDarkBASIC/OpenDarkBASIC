#include "odb-compiler/ast/ArrayRef.hpp"
#include "odb-compiler/ast/ArgList.hpp"
#include "odb-compiler/ast/Identifier.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Variable.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
ArrayRef::ArrayRef(Identifier* identifier, ArgList* args, SourceLocation* location) :
    LValue(location),
    identifier_(identifier),
    args_(args),
    variable_(nullptr)
{
}

// ----------------------------------------------------------------------------
Identifier* ArrayRef::identifier() const
{
    return identifier_;
}

// ----------------------------------------------------------------------------
ArgList* ArrayRef::args() const
{
    return args_;
}

// ----------------------------------------------------------------------------
Type ArrayRef::getType() const
{
    return variable_ ? variable_->getType() : Type::getUnknown();
}

// ----------------------------------------------------------------------------
void ArrayRef::setVariable(Variable* variable)
{
    assert(identifier_->name() == variable->name());
    variable_ = variable;
}

// ----------------------------------------------------------------------------
Variable* ArrayRef::variable() const
{
    assert(identifier_->name() == variable_->name());
    return variable_;
}

// ----------------------------------------------------------------------------
std::string ArrayRef::toString() const
{
    return "ArrayRef";
}

// ----------------------------------------------------------------------------
void ArrayRef::accept(Visitor* visitor)
{
    visitor->visitArrayRef(this);
}
void ArrayRef::accept(ConstVisitor* visitor) const
{
    visitor->visitArrayRef(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange ArrayRef::children()
{
    return {identifier_, args_};
}

// ----------------------------------------------------------------------------
void ArrayRef::swapChild(const Node* oldNode, Node* newNode)
{
    if (identifier_ == oldNode)
        identifier_ = dynamic_cast<Identifier*>(newNode);
    else if (args_ == oldNode)
        args_ = dynamic_cast<ArgList*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* ArrayRef::duplicateImpl() const
{
    return new ArrayRef(identifier_->duplicate<Identifier>(),
        args_->duplicate<ArgList>(),
        location());
}

}
