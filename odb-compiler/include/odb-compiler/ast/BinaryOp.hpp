#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/BinaryOperators.hpp"
#include "odb-compiler/ast/Expression.hpp"

namespace odb {
namespace ast {

class BinaryOp : public Expression
{
public:
    BinaryOp(Expression* lhs, Expression* rhs, SourceLocation* location);

    Expression* lhs() const;
    Expression* rhs() const;

protected:
    Reference<Expression> lhs_;
    Reference<Expression> rhs_;
};

#define X(op, tok)                                                            \
class BinaryOp##op : public BinaryOp                                          \
{                                                                             \
public:                                                                       \
    BinaryOp##op(Expression* lhs, Expression* rhs, SourceLocation* location); \
                                                                              \
    void accept(Visitor* visitor) const override;                             \
};
ODB_BINARY_OP_LIST
#undef X

}
}
