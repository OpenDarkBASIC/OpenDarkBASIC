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
    ~ExpressionList();

    void appendExpression(Expression* expr);

    const std::vector<Reference<Expression>>& expressions() const;

    void accept(Visitor* visitor) const override;

private:
    std::vector<Reference<Expression>> expressions_;
};

}
}
