#pragma once

#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/Type.hpp"

namespace odb::ast {

class ODBCOMPILER_PUBLIC_API ImplicitCast final : public Expression
{
public:
    ImplicitCast(Program* program, SourceLocation* location, Expression* expr, Type targetType);

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
    Type targetType_;
};

}
