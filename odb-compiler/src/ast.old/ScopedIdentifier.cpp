#include "odb-compiler/ast/ScopedIdentifier.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
ScopedIdentifier::ScopedIdentifier(Program* program, SourceLocation* location, Scope scope, std::string name, Annotation annotation) :
    Identifier(program, location, std::move(name), annotation),
    scope_(scope)
{
}

// ----------------------------------------------------------------------------
Scope ScopedIdentifier::scope() const
{
    return scope_;
}

// ----------------------------------------------------------------------------
void ScopedIdentifier::setScope(Scope scope)
{
    scope_ = scope;
}

// ----------------------------------------------------------------------------
std::string ScopedIdentifier::toString() const
{
    return std::string("ScopedIdentifier(")
         + typeAnnotationEnumString(annotation_) + ", "
         + scopeEnumString(scope_)
         + "): \"" + name_ + "\"";
}

// ----------------------------------------------------------------------------
void ScopedIdentifier::accept(Visitor* visitor)
{
    visitor->visitScopedIdentifier(this);
}
void ScopedIdentifier::accept(ConstVisitor* visitor) const
{
    visitor->visitScopedIdentifier(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange ScopedIdentifier::children()
{
    return {};
}

// ----------------------------------------------------------------------------
void ScopedIdentifier::swapChild(const Node* oldNode, Node* newNode)
{
    assert(false);
}

// ----------------------------------------------------------------------------
Node* ScopedIdentifier::duplicateImpl() const
{
    return new ScopedIdentifier(program(), location(), scope_, name_, annotation_);
}

}
