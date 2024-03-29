#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include <vector>

namespace odb::ast {

class Expression;

class ODBCOMPILER_PUBLIC_API InitializerList final : public Expression
{
public:
    InitializerList(Program* program, SourceLocation* location);
    InitializerList(Program* program, SourceLocation* location, Expression* expr);

    void appendExpression(Expression* expr);

    const std::vector<Reference<Expression>>& expressions() const;

    void setTypeBeingInitialized(Type typeBeingInitialized);
    Type getType() const override;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    std::vector<Reference<Expression>> expressions_;
    Type typeBeingInitialized_;
};

}

