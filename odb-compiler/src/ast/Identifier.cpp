#include "odb-compiler/ast/Identifier.hpp"

#include <utility>
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
Identifier::Identifier(std::string  name, SourceLocation* location) :
    Expression(location),
    name_(std::move(name)),
    annotation_(Annotation::NONE)
{
}

// ----------------------------------------------------------------------------
Identifier::Identifier(std::string name, Annotation annotation, SourceLocation* location) :
    Expression(location),
    name_(std::move(name)),
    annotation_(annotation)
{
}

// ----------------------------------------------------------------------------
const std::string& Identifier::name() const
{
    return name_;
}

// ----------------------------------------------------------------------------
Annotation Identifier::annotation() const
{
    return annotation_;
}

// ----------------------------------------------------------------------------
std::string Identifier::toString() const
{
    return std::string("Identifier(")
           + typeAnnotationEnumString(annotation_)
           + "): \"" + name_ + "\"";
}

// ----------------------------------------------------------------------------
void Identifier::accept(Visitor* visitor)
{
    visitor->visitIdentifier(this);
}
void Identifier::accept(ConstVisitor* visitor) const
{
    visitor->visitIdentifier(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange Identifier::children()
{
    return {};
}

// ----------------------------------------------------------------------------
void Identifier::swapChild(const Node* oldNode, Node* newNode)
{
    assert(false);
}

// ----------------------------------------------------------------------------
Node* Identifier::duplicateImpl() const
{
    return new Identifier(name_, annotation_, location());
}

}
