#include "odb-compiler/ast/AnnotatedSymbol.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
AnnotatedSymbol::AnnotatedSymbol(Annotation annotation, const std::string& name, SourceLocation* location) :
    Symbol(name, location),
    annotation_(annotation)
{
}

// ----------------------------------------------------------------------------
Annotation AnnotatedSymbol::annotation() const
{
    return annotation_;
}

// ----------------------------------------------------------------------------
std::string AnnotatedSymbol::toString() const
{
    return typeAnnotationEnumString(annotation_);
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

}
