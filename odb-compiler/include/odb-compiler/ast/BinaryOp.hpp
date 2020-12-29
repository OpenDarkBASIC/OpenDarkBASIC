#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Operators.hpp"
#include "odb-compiler/ast/Expression.hpp"

namespace odb {
namespace ast {

class ODBCOMPILER_PUBLIC_API BinaryOp : public Expression
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
class ODBCOMPILER_PUBLIC_API BinaryOp##op : public BinaryOp                   \
{                                                                             \
public:                                                                       \
    BinaryOp##op(Expression* lhs, Expression* rhs, SourceLocation* location); \
                                                                              \
    void accept(Visitor* visitor) override;                                   \
    void accept(ConstVisitor* visitor) const override;                        \
    void swapChild(const Node* oldNode, Node* newNode) override;              \
};
ODB_BINARY_OP_LIST
#undef X

}
}
