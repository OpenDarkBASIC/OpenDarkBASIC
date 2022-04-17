#include "odb-compiler/ast/Label.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Identifier.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
Label::Label(Identifier* identifier, SourceLocation* location) :
    Statement(location), identifier_(identifier)
{
}

// ----------------------------------------------------------------------------
Identifier* Label::identifier() const
{
    return identifier_;
}

// ----------------------------------------------------------------------------
std::string Label::toString() const
{
    return "Label";
}

// ----------------------------------------------------------------------------
void Label::accept(Visitor* visitor)
{
    visitor->visitLabel(this);
}
void Label::accept(ConstVisitor* visitor) const
{
    visitor->visitLabel(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange Label::children()
{
    return {identifier_};
}

// ----------------------------------------------------------------------------
void Label::swapChild(const Node* oldNode, Node* newNode)
{
    if (identifier_ == oldNode)
        identifier_ = dynamic_cast<Identifier*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* Label::duplicateImpl() const
{
    return new Label(identifier_->duplicate<Identifier>(),
        location());
}

}
