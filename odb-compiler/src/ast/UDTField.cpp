#include "odb-compiler/ast/UDTField.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
UDTFieldOuter::UDTFieldOuter(Expression* left, LValue* right, SourceLocation* location)
    : LValue(location)
    , left_(left)
    , right_(right)
{
}

// ----------------------------------------------------------------------------
Expression* UDTFieldOuter::left() const
{
    return left_;
}

// ----------------------------------------------------------------------------
LValue* UDTFieldOuter::right() const
{
    return right_;
}

// ----------------------------------------------------------------------------
std::string UDTFieldOuter::toString() const
{
    return "UDTFieldOuter";
}

// ----------------------------------------------------------------------------
void UDTFieldOuter::accept(Visitor* visitor)
{
    visitor->visitUDTFieldOuter(this);
}
void UDTFieldOuter::accept(ConstVisitor* visitor) const
{
    visitor->visitUDTFieldOuter(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange UDTFieldOuter::children()
{
    return {left_, right_};
}

// ----------------------------------------------------------------------------
void UDTFieldOuter::swapChild(const Node* oldNode, Node* newNode)
{
    if (left_ == oldNode)
        left_ = dynamic_cast<Expression*>(newNode);
    else if (right_ == oldNode)
        right_ = dynamic_cast<LValue*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* UDTFieldOuter::duplicateImpl() const
{
    return new UDTFieldOuter(
        left_->duplicate<Expression>(),
        right_->duplicate<LValue>(),
        location());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
UDTFieldInner::UDTFieldInner(LValue* left, LValue* right, SourceLocation* location)
    : LValue(location)
    , left_(left)
    , right_(right)
{
}

// ----------------------------------------------------------------------------
LValue* UDTFieldInner::left() const
{
    return left_;
}

// ----------------------------------------------------------------------------
LValue* UDTFieldInner::right() const
{
    return right_;
}

// ----------------------------------------------------------------------------
std::string UDTFieldInner::toString() const
{
    return "UDTFieldInner";
}

// ----------------------------------------------------------------------------
void UDTFieldInner::accept(Visitor* visitor)
{
    visitor->visitUDTFieldInner(this);
}
void UDTFieldInner::accept(ConstVisitor* visitor) const
{
    visitor->visitUDTFieldInner(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange UDTFieldInner::children()
{
    return {left_, right_};
}

// ----------------------------------------------------------------------------
void UDTFieldInner::swapChild(const Node* oldNode, Node* newNode)
{
    if (left_ == oldNode)
        left_ = dynamic_cast<LValue*>(newNode);
    else if (right_ == oldNode)
        right_ = dynamic_cast<LValue*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* UDTFieldInner::duplicateImpl() const
{
    return new UDTFieldInner(
        left_->duplicate<LValue>(),
        right_->duplicate<LValue>(),
        location());
}

}
