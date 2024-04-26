#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Operators.hpp"
#include "odb-compiler/ast/Expression.hpp"

namespace odb::ast {

class ODBCOMPILER_PUBLIC_API BinaryOp final : public Expression
{
public:
    BinaryOp(Program* program, SourceLocation* location, BinaryOpType op, Expression* lhs, Expression* rhs);

    BinaryOpType op() const;
    Expression* lhs() const;
    Expression* rhs() const;

    Type getType() const override;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Expression> lhs_;
    Reference<Expression> rhs_;
    BinaryOpType op_;
};

}
