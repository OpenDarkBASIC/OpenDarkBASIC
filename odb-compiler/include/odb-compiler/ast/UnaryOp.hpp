#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Operators.hpp"
#include "odb-compiler/ast/Expression.hpp"

namespace odb::ast {

class ODBCOMPILER_PUBLIC_API UnaryOp : public Expression
{
public:
    enum Op
    {
#define X(op, tok) op,
        ODB_UNARY_OP_LIST
#undef X
    };

    UnaryOp(Op op, Expression* expr, SourceLocation* location);

    Op op() const;
    Expression* expr() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Expression> expr_;
    Op op_;
};

}
