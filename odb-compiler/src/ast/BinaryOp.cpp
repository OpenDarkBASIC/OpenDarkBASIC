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
void BinaryOp##op::accept(Visitor* visitor) const                             \
{                                                                             \
    visitor->visitBinaryOp##op(this);                                         \
    lhs_->accept(visitor);                                                    \
    rhs_->accept(visitor);                                                    \
}
ODB_BINARY_OP_LIST
#undef X

}
}
