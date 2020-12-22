#include "odb-compiler/ast/UnaryOp.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
UnaryOp::UnaryOp(Expression* expr, SourceLocation* location) :
    Expression(location),
    expr_(expr)
{
    expr->setParent(this);
}

// ----------------------------------------------------------------------------
Expression* UnaryOp::expr() const
{
    return expr_;
}

// ----------------------------------------------------------------------------
#define X(op, tok)                                                            \
UnaryOp##op::UnaryOp##op(Expression* expr, SourceLocation* location) :        \
    UnaryOp(expr, location)                                                   \
{                                                                             \
}                                                                             \
void UnaryOp##op::accept(Visitor* visitor) const                              \
{                                                                             \
    visitor->visitUnaryOp##op(this);                                          \
    expr_->accept(visitor);                                                   \
}
ODB_UNARY_OP_LIST
#undef X

}
}
