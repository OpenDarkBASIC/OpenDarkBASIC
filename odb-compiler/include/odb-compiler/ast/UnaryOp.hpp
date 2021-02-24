#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Operators.hpp"
#include "odb-compiler/ast/Expression.hpp"

namespace odb::ast {

class ODBCOMPILER_PUBLIC_API UnaryOp : public Expression
{
public:
    UnaryOp(UnaryOpType op, Expression* expr, SourceLocation* location);

    UnaryOpType op() const;
    Expression* expr() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Expression> expr_;
    UnaryOpType op_;
};

}
