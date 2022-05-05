#include "odb-compiler/ast/ImplicitCast.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
ImplicitCast::ImplicitCast(Expression* expr, Type targetType, SourceLocation* location) :
    Expression(location),
    expr_(expr),
    targetType_(targetType)
{
}

// ----------------------------------------------------------------------------
Expression* ImplicitCast::expr() const
{
    return expr_;
}

// ----------------------------------------------------------------------------
Type ImplicitCast::getType() const
{
    return targetType_;
}

// ----------------------------------------------------------------------------
std::string ImplicitCast::toString() const
{
    return "ImplicitCast " + targetType_.toString();
}

// ----------------------------------------------------------------------------
void ImplicitCast::accept(Visitor* visitor)
{
    visitor->visitImplicitCast(this);
}
void ImplicitCast::accept(ConstVisitor* visitor) const
{
    visitor->visitImplicitCast(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange ImplicitCast::children()
{
    return {expr_};
}

// ----------------------------------------------------------------------------
void ImplicitCast::swapChild(const Node* oldNode, Node* newNode)
{
    if (expr_ == oldNode)
        expr_ = dynamic_cast<Expression*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* ImplicitCast::duplicateImpl() const
{
    return new ImplicitCast(
        expr_->duplicate<Expression>(),
        targetType_,
        location());
}

}
