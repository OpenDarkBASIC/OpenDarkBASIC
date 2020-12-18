#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb {
namespace ast {

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
void Symbol::accept(Visitor* visitor) const
{
    visitor->visitSymbol(this);
}

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
void AnnotatedSymbol::accept(Visitor* visitor) const
{
    visitor->visitAnnotatedSymbol(this);
}


// ----------------------------------------------------------------------------
ScopedSymbol::ScopedSymbol(Scope scope, const std::string& name, SourceLocation* location) :
    Symbol(name, location),
    scope_(scope)
{
}

// ----------------------------------------------------------------------------
Symbol::Scope ScopedSymbol::scope() const
{
    return scope_;
}

// ----------------------------------------------------------------------------
void ScopedSymbol::accept(Visitor* visitor) const
{
    visitor->visitScopedSymbol(this);
}

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
void ScopedAnnotatedSymbol::accept(Visitor* visitor) const
{
    visitor->visitScopedAnnotatedSymbol(this);
}

}
}
