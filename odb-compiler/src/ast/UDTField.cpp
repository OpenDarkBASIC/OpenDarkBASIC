#include "odb-compiler/ast/UDTField.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
UDTFieldOuter::UDTFieldOuter(Expression* left, LValue* right, SourceLocation* location)
    : LValue(location)
    , left_(left)
    , right_(right)
{
    left->setParent(this);
    right->setParent(this);
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
void UDTFieldOuter::accept(Visitor* visitor)
{
    visitor->visitUDTFieldOuter(this);
    left_->accept(visitor);
    right_->accept(visitor);
}
void UDTFieldOuter::accept(ConstVisitor* visitor) const
{
    visitor->visitUDTFieldOuter(this);
    left_->accept(visitor);
    right_->accept(visitor);
}

// ----------------------------------------------------------------------------
void UDTFieldOuter::swapChild(const Node* oldNode, Node* newNode)
{
    if (left_ == oldNode)
        left_ = dynamic_cast<Expression*>(newNode);
    else if (right_ == oldNode)
        right_ = dynamic_cast<LValue*>(newNode);
    else
        assert(false),

    newNode->setParent(this);
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
    left->setParent(this);
    right->setParent(this);
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
void UDTFieldInner::accept(Visitor* visitor)
{
    visitor->visitUDTFieldInner(this);
    left_->accept(visitor);
    right_->accept(visitor);
}
void UDTFieldInner::accept(ConstVisitor* visitor) const
{
    visitor->visitUDTFieldInner(this);
    left_->accept(visitor);
    right_->accept(visitor);
}

// ----------------------------------------------------------------------------
void UDTFieldInner::swapChild(const Node* oldNode, Node* newNode)
{
    if (left_ == oldNode)
        left_ = dynamic_cast<LValue*>(newNode);
    else if (right_ == oldNode)
        right_ = dynamic_cast<LValue*>(newNode);
    else
        assert(false),

    newNode->setParent(this);
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
