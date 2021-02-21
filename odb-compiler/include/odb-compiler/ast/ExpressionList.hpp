#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Node.hpp"
#include <vector>

namespace odb {
namespace ast {

class Expression;

class ODBCOMPILER_PUBLIC_API ExpressionList : public Node
{
public:
    ExpressionList(SourceLocation* location);
    ExpressionList(Expression* expr, SourceLocation* location);

    void appendExpression(Expression* expr);

    const std::vector<Reference<Expression>>& expressions() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    std::vector<Reference<Expression>> expressions_;
};

}
}
