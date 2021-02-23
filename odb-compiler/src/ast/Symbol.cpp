#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
Symbol::Symbol(const std::string& name, SourceLocation* location) :
    Node(location),
    name_(name)
{
}

// ----------------------------------------------------------------------------
const std::string& Symbol::name() const
{
    return name_;
}

// ----------------------------------------------------------------------------
void Symbol::accept(Visitor* visitor)
{
    visitor->visitSymbol(this);
}
void Symbol::accept(ConstVisitor* visitor) const
{
    visitor->visitSymbol(this);
}

// ----------------------------------------------------------------------------
void Symbol::swapChild(const Node* oldNode, Node* newNode)
{
    assert(false);
}

// ----------------------------------------------------------------------------
Node* Symbol::duplicateImpl() const
{
    return new Symbol(name_, location());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
AnnotatedSymbol::AnnotatedSymbol(Annotation annotation, const std::string& name, SourceLocation* location) :
    Symbol(name, location),
    annotation_(annotation)
{
}

// ----------------------------------------------------------------------------
Symbol::Annotation AnnotatedSymbol::annotation() const
{
    return annotation_;
}

// ----------------------------------------------------------------------------
void AnnotatedSymbol::accept(Visitor* visitor)
{
    visitor->visitAnnotatedSymbol(this);
}
void AnnotatedSymbol::accept(ConstVisitor* visitor) const
{
    visitor->visitAnnotatedSymbol(this);
}

// ----------------------------------------------------------------------------
void AnnotatedSymbol::swapChild(const Node* oldNode, Node* newNode)
{
    assert(false);
}

// ----------------------------------------------------------------------------
Node* AnnotatedSymbol::duplicateImpl() const
{
    return new AnnotatedSymbol(annotation_, name_, location());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
ScopedAnnotatedSymbol::ScopedAnnotatedSymbol(Scope scope, Annotation annotation, const std::string& name, SourceLocation* location) :
    Symbol(name, location),
    scope_(scope),
    annotation_(annotation)
{
}

// ----------------------------------------------------------------------------
Symbol::Scope ScopedAnnotatedSymbol::scope() const
{
    return scope_;
}

// ----------------------------------------------------------------------------
Symbol::Annotation ScopedAnnotatedSymbol::annotation() const
{
    return annotation_;
}

// ----------------------------------------------------------------------------
void ScopedAnnotatedSymbol::setScope(Scope scope)
{
    scope_ = scope;
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
