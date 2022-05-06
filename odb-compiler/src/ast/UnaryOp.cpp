#include "odb-compiler/ast/UnaryOp.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
UnaryOp::UnaryOp(UnaryOpType op, Expression* expr, SourceLocation* location) :
    Expression(location),
    expr_(expr),
    op_(op)
{
}

// ----------------------------------------------------------------------------
UnaryOpType UnaryOp::op() const
{
    return op_;
}

// ----------------------------------------------------------------------------
Expression* UnaryOp::expr() const
{
    return expr_;
}

// ----------------------------------------------------------------------------
Type UnaryOp::getType() const
{
    return expr_->getType();
}

// ----------------------------------------------------------------------------
std::string UnaryOp::toString() const
{
    return std::string("UnaryOp(") + unaryOpTypeEnumString(op_) + ")";
}

// ----------------------------------------------------------------------------
void UnaryOp::accept(Visitor* visitor)
{
    visitor->visitUnaryOp(this);
}
void UnaryOp::accept(ConstVisitor* visitor) const
{
    visitor->visitUnaryOp(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange UnaryOp::children()
{
    return {expr_};
}

// ----------------------------------------------------------------------------
void UnaryOp::swapChild(const Node* oldNode, Node* newNode)
{
    if (expr_ == oldNode)
        expr_ = dynamic_cast<Expression*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* UnaryOp::duplicateImpl() const
{
    return new UnaryOp(
        op_,
        expr_->duplicate<Expression>(),
        location());
}

}
