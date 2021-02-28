#include "odb-compiler/ast/ScopedAnnotatedSymbol.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
ScopedAnnotatedSymbol::ScopedAnnotatedSymbol(Scope scope, Annotation annotation, const std::string& name, SourceLocation* location) :
    Symbol(name, location),
    scope_(scope),
    annotation_(annotation)
{
}

// ----------------------------------------------------------------------------
Scope ScopedAnnotatedSymbol::scope() const
{
    return scope_;
}

// ----------------------------------------------------------------------------
Annotation ScopedAnnotatedSymbol::annotation() const
{
    return annotation_;
}

// ----------------------------------------------------------------------------
void ScopedAnnotatedSymbol::setScope(Scope scope)
{
    scope_ = scope;
}

// ----------------------------------------------------------------------------
std::string ScopedAnnotatedSymbol::toString() const
{
    return std::string("ScopedAnnotatedSymbol(")
         + typeAnnotationEnumString(annotation_) + ", "
         + scopeEnumString(scope_)
         + "): \"" + name_ + "\"";
}

// ----------------------------------------------------------------------------
void ScopedAnnotatedSymbol::accept(Visitor* visitor)
{
    visitor->visitScopedAnnotatedSymbol(this);
}
void ScopedAnnotatedSymbol::accept(ConstVisitor* visitor) const
{
    visitor->visitScopedAnnotatedSymbol(this);
}

// ----------------------------------------------------------------------------
void ScopedAnnotatedSymbol::swapChild(const Node* oldNode, Node* newNode)
{
    assert(false);
}

// ----------------------------------------------------------------------------
Node* ScopedAnnotatedSymbol::duplicateImpl() const
{
    return new ScopedAnnotatedSymbol(scope_, annotation_, name_, location());
}

}
