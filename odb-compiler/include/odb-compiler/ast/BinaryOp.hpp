#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Operators.hpp"
#include "odb-compiler/ast/Expression.hpp"

namespace odb::ast {

class ODBCOMPILER_PUBLIC_API BinaryOp : public Expression
{
public:
    enum Op
    {
#define X(op, tok) op,
        ODB_BINARY_OP_LIST
#undef X
    };
    BinaryOp(Op op, Expression* lhs, Expression* rhs, SourceLocation* location);

    Op op() const;
    Expression* lhs() const;
    Expression* rhs() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Expression> lhs_;
    Reference<Expression> rhs_;
    Op op_;
};

}
