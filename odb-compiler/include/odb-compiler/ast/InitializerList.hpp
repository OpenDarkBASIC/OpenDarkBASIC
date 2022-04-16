#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include <vector>

namespace odb::ast {

class Expression;

class ODBCOMPILER_PUBLIC_API InitializerList final : public Expression
{
public:
    InitializerList(SourceLocation* location);
    InitializerList(Expression* expr, SourceLocation* location);

    void appendExpression(Expression* expr);

    const std::vector<Reference<Expression>>& expressions() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    std::vector<Reference<Expression>> expressions_;
};

}

