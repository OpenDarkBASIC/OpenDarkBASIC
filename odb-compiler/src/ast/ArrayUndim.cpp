#include "odb-compiler/ast/ArgList.hpp"
#include "odb-compiler/ast/ArrayUndim.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/ScopedIdentifier.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Variable.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
ArrayUndim::ArrayUndim(Program* program, SourceLocation* location, ScopedIdentifier* identifier, ArgList* dims)
    : Statement(program, location)
    , identifier_(identifier)
    , dims_(dims)
{
}

// ----------------------------------------------------------------------------
ArrayUndim::ArrayUndim(Program* program, SourceLocation* location, ScopedIdentifier* identifier)
    : Statement(program, location)
    , identifier_(identifier)
    , dims_(nullptr)
{
}

// ----------------------------------------------------------------------------
ScopedIdentifier* ArrayUndim::identifier() const
{
    return identifier_;
}

// ----------------------------------------------------------------------------
MaybeNull<ArgList> ArrayUndim::dims() const
{
    return dims_.get();
}

// ----------------------------------------------------------------------------
void ArrayUndim::setVariable(Variable* variable)
{
    assert(identifier_->name() == variable->name());
    variable_ = variable;
}

// ----------------------------------------------------------------------------
Variable* ArrayUndim::variable() const
{
    assert(identifier_->name() == variable_->name());
    return variable_;
}

// ----------------------------------------------------------------------------
std::string ArrayUndim::toString() const
{
    return "ArrayUndim";
}

// ----------------------------------------------------------------------------
void ArrayUndim::accept(Visitor* visitor)
{
    visitor->visitArrayUndim(this);
}

// ----------------------------------------------------------------------------
void ArrayUndim::accept(ConstVisitor* visitor) const
{
    visitor->visitArrayUndim(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange ArrayUndim::children()
{
    Node::ChildRange children;
    children.push_back(identifier_);
    children.push_back(dims_);
    return children;
}

// ----------------------------------------------------------------------------
void ArrayUndim::swapChild(const Node* oldNode, Node* newNode)
{
    if (identifier_ == oldNode)
        identifier_ = dynamic_cast<ScopedIdentifier*>(newNode);
    else if (dims_ == oldNode)
        dims_ = dynamic_cast<ArgList*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* ArrayUndim::duplicateImpl() const
{
    return new ArrayUndim(
        program(),
        location(),
        identifier_->duplicate<ScopedIdentifier>(),
        dims_->duplicate<ArgList>());
}

}
