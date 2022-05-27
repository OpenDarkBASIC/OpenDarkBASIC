#include "odb-compiler/ast/BinaryOp.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
BinaryOp::BinaryOp(Program* program, SourceLocation* location, BinaryOpType op, Expression* lhs, Expression* rhs)
    : Expression(program, location)
    , lhs_(lhs)
    , rhs_(rhs)
    , op_(op)
{
}

// ----------------------------------------------------------------------------
BinaryOpType BinaryOp::op() const
{
    return op_;
}

// ----------------------------------------------------------------------------
Expression* BinaryOp::lhs() const
{
    return lhs_;
}

// ----------------------------------------------------------------------------
Expression* BinaryOp::rhs() const
{
    return rhs_;
}

// ----------------------------------------------------------------------------
Type BinaryOp::getType() const
{
    switch (op_)
    {
    // Comparison operators are always boolean.
    case ast::BinaryOpType::LESS_THAN:
    case ast::BinaryOpType::LESS_EQUAL:
    case ast::BinaryOpType::GREATER_THAN:
    case ast::BinaryOpType::GREATER_EQUAL:
    case ast::BinaryOpType::EQUAL:
    case ast::BinaryOpType::NOT_EQUAL:
        return Type::getBuiltin(BuiltinType::Boolean);

    // Otherwise, they are the common type of the LHS and RHS.
    default:
        if (lhs_->getType() == rhs_->getType())
        {
            // Types should've been unified by adding implicit casts.
            return lhs_->getType();
        }
        else
        {
            // Until we've unified the types, we don't know which is the right one.
            return Type::getUnknown();
        }
    }
}

// ----------------------------------------------------------------------------
std::string BinaryOp::toString() const
{
    return std::string("BinaryOp(") + binaryOpTypeEnumString(op_) + ")";
}

// ----------------------------------------------------------------------------
void BinaryOp::accept(Visitor* visitor)
{
    visitor->visitBinaryOp(this);
}
void BinaryOp::accept(ConstVisitor* visitor) const
{
    visitor->visitBinaryOp(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange BinaryOp::children()
{
    return {lhs_, rhs_};
}

// ----------------------------------------------------------------------------
void BinaryOp::swapChild(const Node* oldNode, Node* newNode)
{
    if (lhs_ == oldNode)
        lhs_ = dynamic_cast<Expression*>(newNode);
    else if (rhs_ == oldNode)
        rhs_ = dynamic_cast<Expression*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* BinaryOp::duplicateImpl() const
{
    return new BinaryOp(
        program(),
        location(),
        op_,
        lhs_->duplicate<Expression>(),
        rhs_->duplicate<Expression>());
}

}
