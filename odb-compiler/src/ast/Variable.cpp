#include <utility>

#include "odb-compiler/ast/Variable.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
Variable::Variable(SourceLocation* location, std::string name, Type type) :
    Node(location),
    name_(std::move(name)),
    annotation_(Annotation::NONE),
    type_(std::move(type))
{
}

// ----------------------------------------------------------------------------
Variable::Variable(SourceLocation* location, std::string name, Annotation annotation, Type type) :
    Node(location),
    name_(std::move(name)),
    annotation_(annotation),
    type_(std::move(type))
{
}

// ----------------------------------------------------------------------------
const std::string& Variable::name() const
{
    return name_;
}

// ----------------------------------------------------------------------------
Annotation Variable::annotation() const
{
    return annotation_;
}

// ----------------------------------------------------------------------------
Type Variable::type() const
{
    return type_;
}

// ----------------------------------------------------------------------------
std::string Variable::toString() const
{
    return std::string("Variable(")
           + typeAnnotationEnumString(annotation_)
           + "): \"" + name_ + "\"";
}

// ----------------------------------------------------------------------------
void Variable::accept(Visitor* visitor)
{
    visitor->visitVariable(this);
}
void Variable::accept(ConstVisitor* visitor) const
{
    visitor->visitVariable(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange Variable::children()
{
    return {};
}

// ----------------------------------------------------------------------------
void Variable::swapChild(const Node* oldNode, Node* newNode)
{
    assert(false);
}

// ----------------------------------------------------------------------------
Node* Variable::duplicateImpl() const
{
    assert(false);
    return nullptr;
}

}
