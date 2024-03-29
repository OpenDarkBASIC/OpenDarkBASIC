#include "odb-compiler/ast/ArrayDecl.hpp"
#include "odb-compiler/ast/ArgList.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/ScopedIdentifier.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Variable.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
ArrayDecl::ArrayDecl(Program* program, SourceLocation* location, ScopedIdentifier* identifier, Type type, ArgList* dims)
    : Statement(program, location)
    , identifier_(identifier)
    , type_(std::move(type))
    , dims_(dims)
{
}

// ----------------------------------------------------------------------------
ArrayDecl::ArrayDecl(Program* program, SourceLocation* location, ScopedIdentifier* identifier, Type type)
    : Statement(program, location)
    , identifier_(identifier)
    , type_(std::move(type))
    , dims_(nullptr)
{
}

// ----------------------------------------------------------------------------
ScopedIdentifier* ArrayDecl::identifier() const
{
    return identifier_;
}

// ----------------------------------------------------------------------------
Type ArrayDecl::type() const
{
    return type_;
}

// ----------------------------------------------------------------------------
MaybeNull<ArgList> ArrayDecl::dims() const
{
    return dims_.get();
}

// ----------------------------------------------------------------------------
void ArrayDecl::setVariable(Variable* variable)
{
    assert(identifier_->name() == variable->name());
    variable_ = variable;
}

// ----------------------------------------------------------------------------
Variable* ArrayDecl::variable() const
{
    assert(identifier_->name() == variable_->name());
    return variable_;
}

// ----------------------------------------------------------------------------
std::string ArrayDecl::toString() const
{
    return "ArrayDecl";
}

// ----------------------------------------------------------------------------
void ArrayDecl::accept(Visitor* visitor)
{
    visitor->visitArrayDecl(this);
}

// ----------------------------------------------------------------------------
void ArrayDecl::accept(ConstVisitor* visitor) const
{
    visitor->visitArrayDecl(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange ArrayDecl::children()
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
void ArrayDecl::swapChild(const Node* oldNode, Node* newNode)
{
    if (identifier_ == oldNode)
        identifier_ = dynamic_cast<ScopedIdentifier*>(newNode);
    else if (dims_ == oldNode)
        dims_ = dynamic_cast<ArgList*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* ArrayDecl::duplicateImpl() const
{
    return new ArrayDecl(
        program(),
        location(),
        identifier_->duplicate<ScopedIdentifier>(),
        type_,
        dims_->duplicate<ArgList>());
}

}
