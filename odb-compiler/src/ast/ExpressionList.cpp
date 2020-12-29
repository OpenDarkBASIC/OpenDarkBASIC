#include "odb-compiler/ast/ExpressionList.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
ExpressionList::ExpressionList(SourceLocation* location) :
    Node(location)
{
}

// ----------------------------------------------------------------------------
ExpressionList::~ExpressionList() = default;

// ----------------------------------------------------------------------------
void ExpressionList::appendExpression(Expression* expr)
{
    expr->setParent(this);
    expressions_.push_back(expr);
}

// ----------------------------------------------------------------------------
const std::vector<Reference<Expression>>& ExpressionList::expressions() const
{
    return expressions_;
}

// ----------------------------------------------------------------------------
void ExpressionList::accept(Visitor* visitor) const
{
    visitor->visitExpressionList(this);
    for (const auto& expr : expressions_)
        expr->accept(visitor);
}

}
}
