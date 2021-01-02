#include "odb-compiler/ast/ArrayRef.hpp"
#include "odb-compiler/ast/Assignment.hpp"
#include "odb-compiler/ast/ExpressionList.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/ast/UDTArrayRef.hpp"
#include "odb-compiler/ast/UDTVarRef.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
Assignment::Assignment(LValue* lvalue, SourceLocation* location) :
    Statement(location),
    lvalue_(lvalue)
{
    lvalue->setParent(this);
}

// ----------------------------------------------------------------------------
VarAssignment::VarAssignment(VarRef* var, Expression* expr, SourceLocation* location) :
    Assignment(var, location),
    expr_(expr)
{
    expr->setParent(this);
}

// ----------------------------------------------------------------------------
VarRef* VarAssignment::variable() const
{
    return static_cast<VarRef*>(lvalue_.get());
}

// ----------------------------------------------------------------------------
Expression* VarAssignment::expression() const
{
    return expr_;
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
        lvalue_ = dynamic_cast<LValue*>(newNode);
    else if (expr_ == oldNode)
        expr_ = dynamic_cast<Expression*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
ArrayAssignment::ArrayAssignment(ArrayRef* var, Expression* expr, SourceLocation* location) :
    Assignment(var, location),
    expr_(expr)
{
    expr->setParent(this);
}

// ----------------------------------------------------------------------------
ArrayRef* ArrayAssignment::array() const
{
    return static_cast<ArrayRef*>(lvalue_.get());
}

// ----------------------------------------------------------------------------
Expression* ArrayAssignment::expression() const
{
    return expr_;
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
        lvalue_ = dynamic_cast<LValue*>(newNode);
    else if (expr_ == oldNode)
        expr_ = dynamic_cast<Expression*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
UDTVarAssignment::UDTVarAssignment(UDTVarRef* var, Expression* expr, SourceLocation* location) :
    Assignment(var, location),
    expr_(expr)
{
    expr->setParent(this);
}

// ----------------------------------------------------------------------------
UDTVarRef* UDTVarAssignment::variable() const
{
    return static_cast<UDTVarRef*>(lvalue_.get());
}

// ----------------------------------------------------------------------------
Expression* UDTVarAssignment::expression() const
{
    return expr_;
}

// ----------------------------------------------------------------------------
void UDTVarAssignment::accept(Visitor* visitor)
{
    visitor->visitUDTVarAssignment(this);
    lvalue_->accept(visitor);
    expr_->accept(visitor);
}
void UDTVarAssignment::accept(ConstVisitor* visitor) const
{
    visitor->visitUDTVarAssignment(this);
    lvalue_->accept(visitor);
    expr_->accept(visitor);
}

// ----------------------------------------------------------------------------
void UDTVarAssignment::swapChild(const Node* oldNode, Node* newNode)
{
    if (lvalue_ == oldNode)
        lvalue_ = dynamic_cast<LValue*>(newNode);
    else if (expr_ == oldNode)
        expr_ = dynamic_cast<Expression*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
UDTArrayAssignment::UDTArrayAssignment(UDTArrayRef* var, Expression* expr, SourceLocation* location) :
    Assignment(var, location),
    expr_(expr)
{
    expr->setParent(this);
}

// ----------------------------------------------------------------------------
UDTArrayRef* UDTArrayAssignment::array() const
{
    return static_cast<UDTArrayRef*>(lvalue_.get());
}

// ----------------------------------------------------------------------------
Expression* UDTArrayAssignment::expression() const
{
    return expr_;
}

// ----------------------------------------------------------------------------
void UDTArrayAssignment::accept(Visitor* visitor)
{
    visitor->visitUDTArrayAssignment(this);
    lvalue_->accept(visitor);
    expr_->accept(visitor);
}
void UDTArrayAssignment::accept(ConstVisitor* visitor) const
{
    visitor->visitUDTArrayAssignment(this);
    lvalue_->accept(visitor);
    expr_->accept(visitor);
}

// ----------------------------------------------------------------------------
void UDTArrayAssignment::swapChild(const Node* oldNode, Node* newNode)
{
    if (lvalue_ == oldNode)
        lvalue_ = dynamic_cast<LValue*>(newNode);
    else if (expr_ == oldNode)
        expr_ = dynamic_cast<Expression*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

}
