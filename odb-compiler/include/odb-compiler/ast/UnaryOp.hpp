#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Operators.hpp"
#include "odb-compiler/ast/Expression.hpp"

namespace odb {
namespace ast {

class UnaryOp : public Expression
{
public:
    UnaryOp(Expression* expr, SourceLocation* location);

    Expression* expr() const;

protected:
    Reference<Expression> expr_;
};

#define X(op, tok)                                                            \
class UnaryOp##op : public UnaryOp                                            \
{                                                                             \
public:                                                                       \
    UnaryOp##op(Expression* expr, SourceLocation* location);                  \
                                                                              \
    void accept(Visitor* visitor) const override;                             \
};
ODB_UNARY_OP_LIST
#undef X

}
}
