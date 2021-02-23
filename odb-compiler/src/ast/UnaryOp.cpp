#include "odb-compiler/ast/UnaryOp.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
UnaryOp::UnaryOp(Op op, Expression* expr, SourceLocation* location) :
    Expression(location),
    expr_(expr),
    op_(op)
{
    expr->setParent(this);
}

// ----------------------------------------------------------------------------
UnaryOp::Op UnaryOp::op() const
{
    return op_;
}

// ----------------------------------------------------------------------------
Expression* UnaryOp::expr() const
{
    return expr_;
}

// ----------------------------------------------------------------------------
std::string UnaryOp::toString() const
{
    static const char* table[] = {
#define X(op, tok) #op,
        ODB_UNARY_OP_LIST
#undef X
    };
    return std::string("UnaryOp(") + table[op_] + ")";
}

// ----------------------------------------------------------------------------
void UnaryOp::accept(Visitor* visitor)
{
    visitor->visitUnaryOp(this);
    expr_->accept(visitor);
}
void UnaryOp::accept(ConstVisitor* visitor) const
{
    visitor->visitUnaryOp(this);
    expr_->accept(visitor);
}

// ----------------------------------------------------------------------------
void UnaryOp::swapChild(const Node* oldNode, Node* newNode)
{
    if (expr_ == oldNode)
        expr_ = dynamic_cast<Expression*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
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
