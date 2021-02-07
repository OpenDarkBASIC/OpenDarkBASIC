#include "odb-compiler/ast/ArrayRef.hpp"
#include "odb-compiler/ast/Assignment.hpp"
#include "odb-compiler/ast/ExpressionList.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/UDTField.hpp"
#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
Assignment::Assignment(LValue* lvalue, Expression* expr, SourceLocation* location)
    : Statement(location)
    , lvalue_(lvalue)
    , expr_(expr)
{
    lvalue->setParent(this);
    expr->setParent(this);
}

// ----------------------------------------------------------------------------
LValue* Assignment::lvalue() const
{
    return lvalue_;
}

// ----------------------------------------------------------------------------
Expression* Assignment::expression() const
{
    return expr_;
}

// ----------------------------------------------------------------------------
VarAssignment::VarAssignment(VarRef* var, Expression* expr, SourceLocation* location)
    : Assignment(var, expr, location)
{
}

// ----------------------------------------------------------------------------
VarRef* VarAssignment::variable() const
{
    return static_cast<VarRef*>(lvalue_.get());
}

// ----------------------------------------------------------------------------
void VarAssignment::accept(Visitor* visitor)
{
    visitor->visitVarAssignment(this);
    lvalue_->accept(visitor);
    expr_->accept(visitor);
}
void VarAssignment::accept(ConstVisitor* visitor) const
{
    visitor->visitVarAssignment(this);
    lvalue_->accept(visitor);
    expr_->accept(visitor);
}

// ----------------------------------------------------------------------------
void VarAssignment::swapChild(const Node* oldNode, Node* newNode)
{
    if (lvalue_ == oldNode)
        lvalue_ = dynamic_cast<VarRef*>(newNode);
    else if (expr_ == oldNode)
        expr_ = dynamic_cast<Expression*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* VarAssignment::duplicateImpl() const
{
    return new VarAssignment(
        lvalue_->duplicate<VarRef>(),
        expr_->duplicate<Expression>(),
        location());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
ArrayAssignment::ArrayAssignment(ArrayRef* var, Expression* expr, SourceLocation* location)
    : Assignment(var, expr, location)
{
}

// ----------------------------------------------------------------------------
ArrayRef* ArrayAssignment::array() const
{
    return static_cast<ArrayRef*>(lvalue_.get());
}

// ----------------------------------------------------------------------------
void ArrayAssignment::accept(Visitor* visitor)
{
    visitor->visitArrayAssignment(this);
    lvalue_->accept(visitor);
    expr_->accept(visitor);
}
void ArrayAssignment::accept(ConstVisitor* visitor) const
{
    visitor->visitArrayAssignment(this);
    lvalue_->accept(visitor);
    expr_->accept(visitor);
}

// ----------------------------------------------------------------------------
void ArrayAssignment::swapChild(const Node* oldNode, Node* newNode)
{
    if (lvalue_ == oldNode)
        lvalue_ = dynamic_cast<ArrayRef*>(newNode);
    else if (expr_ == oldNode)
        expr_ = dynamic_cast<Expression*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* ArrayAssignment::duplicateImpl() const
{
    return new ArrayAssignment(
        lvalue_->duplicate<ArrayRef>(),
        expr_->duplicate<Expression>(),
        location());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
UDTFieldAssignment::UDTFieldAssignment(UDTFieldOuter* field, Expression* expr, SourceLocation* location)
    : Assignment(field, expr, location)
{
}

// ----------------------------------------------------------------------------
UDTFieldOuter* UDTFieldAssignment::field() const
{
    return static_cast<UDTFieldOuter*>(lvalue_.get());
}

// ----------------------------------------------------------------------------
void UDTFieldAssignment::accept(Visitor* visitor)
{
    visitor->visitUDTFieldAssignment(this);
    lvalue_->accept(visitor);
    expr_->accept(visitor);
}
void UDTFieldAssignment::accept(ConstVisitor* visitor) const
{
    visitor->visitUDTFieldAssignment(this);
    lvalue_->accept(visitor);
    expr_->accept(visitor);
}

// ----------------------------------------------------------------------------
void UDTFieldAssignment::swapChild(const Node* oldNode, Node* newNode)
{
    if (lvalue_ == oldNode)
        lvalue_ = dynamic_cast<UDTFieldOuter*>(newNode);
    else if (expr_ == oldNode)
        expr_ = dynamic_cast<Expression*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* UDTFieldAssignment::duplicateImpl() const
{
    return new UDTFieldAssignment(
        lvalue_->duplicate<UDTFieldOuter>(),
        expr_->duplicate<Expression>(),
        location());
}

}
