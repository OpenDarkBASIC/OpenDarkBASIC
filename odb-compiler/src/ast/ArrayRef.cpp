#include "odb-compiler/ast/ArrayRef.hpp"
#include "odb-compiler/ast/ArgList.hpp"
#include "odb-compiler/ast/Identifier.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Variable.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
ArrayRef::ArrayRef(Program* program, SourceLocation* location, Identifier* identifier, ArgList* dims) :
    LValue(program, location),
    identifier_(identifier),
    dims_(dims),
    variable_(nullptr)
{
}

// ----------------------------------------------------------------------------
ArrayRef::ArrayRef(Program* program, SourceLocation* location, Identifier* identifier) :
    LValue(program, location),
    identifier_(identifier),
    dims_(nullptr),
    variable_(nullptr)
{
}

// ----------------------------------------------------------------------------
Identifier* ArrayRef::identifier() const
{
    return identifier_;
}

// ----------------------------------------------------------------------------
MaybeNull<ArgList> ArrayRef::dims() const
{
    return dims_.get();
}

// ----------------------------------------------------------------------------
Type ArrayRef::getType() const
{
    if (variable_)
    {
        assert(variable_->getType().isArray());
        return *variable_->getType().getArrayInnerType();
    }
    return Type::getUnknown("Unresolved array");
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
    if (dims_)
    {
        return {identifier_, dims_};
    }
    else
    {
        return {identifier_};
    }
}

// ----------------------------------------------------------------------------
void ArrayRef::swapChild(const Node* oldNode, Node* newNode)
{
    if (identifier_ == oldNode)
        identifier_ = dynamic_cast<Identifier*>(newNode);
    else if (dims_ == oldNode)
        dims_ = dynamic_cast<ArgList*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* ArrayRef::duplicateImpl() const
{
    return new ArrayRef(
        program(),
        location(),
        identifier_->duplicate<Identifier>(), dims_->duplicate<ArgList>());
}

}
