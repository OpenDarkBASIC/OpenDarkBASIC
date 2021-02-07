#include "odb-compiler/ast/BinaryOp.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
BinaryOp::BinaryOp(Expression* lhs, Expression* rhs, SourceLocation* location) :
    Expression(location),
    lhs_(lhs),
    rhs_(rhs)
{
    lhs->setParent(this);
    rhs->setParent(this);
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
#define X(op, tok)                                                            \
BinaryOp##op::BinaryOp##op(Expression* lhs, Expression* rhs, SourceLocation* location) : \
    BinaryOp(lhs, rhs, location)                                              \
{                                                                             \
}                                                                             \
void BinaryOp##op::accept(Visitor* visitor)                                   \
{                                                                             \
    visitor->visitBinaryOp##op(this);                                         \
    lhs_->accept(visitor);                                                    \
    rhs_->accept(visitor);                                                    \
}                                                                             \
void BinaryOp##op::accept(ConstVisitor* visitor) const                        \
{                                                                             \
    visitor->visitBinaryOp##op(this);                                         \
    lhs_->accept(visitor);                                                    \
    rhs_->accept(visitor);                                                    \
}                                                                             \
void BinaryOp##op::swapChild(const Node* oldNode, Node* newNode)              \
{                                                                             \
    if (lhs_ == oldNode)                                                      \
        lhs_ = dynamic_cast<Expression*>(newNode);                            \
    else if (rhs_ == oldNode)                                                 \
        rhs_ = dynamic_cast<Expression*>(newNode);                            \
    else                                                                      \
        assert(false);                                                        \
                                                                              \
    newNode->setParent(this);                                                 \
}                                                                             \
Node* BinaryOp##op::duplicateImpl() const                                     \
{                                                                             \
    return new BinaryOp##op(                                                  \
        lhs_->duplicate<Expression>(),                                        \
        rhs_->duplicate<Expression>(),                                        \
        location());                                                          \
}
ODB_BINARY_OP_LIST
#undef X

}
}
