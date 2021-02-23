#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Node.hpp"
#include <vector>

namespace odb::ast {

class Expression;

class ODBCOMPILER_PUBLIC_API ArgList : public Node
{
public:
    ArgList(SourceLocation* location);
    ArgList(Expression* expr, SourceLocation* location);

    void appendExpression(Expression* expr);

    const std::vector<Reference<Expression>>& expressions() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    std::vector<Reference<Expression>> expressions_;
};

}
