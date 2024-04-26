#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Operators.hpp"
#include "odb-compiler/ast/Expression.hpp"

namespace odb::ast {

class ODBCOMPILER_PUBLIC_API UnaryOp final : public Expression
{
public:
    UnaryOp(Program* program, SourceLocation* location, UnaryOpType op, Expression* expr);

    UnaryOpType op() const;
    Expression* expr() const;

    Type getType() const override;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Expression> expr_;
    UnaryOpType op_;
};

}
