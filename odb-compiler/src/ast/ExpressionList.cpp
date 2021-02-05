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
ExpressionList::ExpressionList(Expression* expr, SourceLocation* location) :
    Node(location)
{
    appendExpression(expr);
}

// ----------------------------------------------------------------------------
ExpressionList::~ExpressionList() = default;

// ----------------------------------------------------------------------------
void ExpressionList::appendExpression(Expression* expr)
{
    expr->setParent(this);
    expressions_.push_back(expr);

    location()->unionize(expr->location());
}

// ----------------------------------------------------------------------------
const std::vector<Reference<Expression>>& ExpressionList::expressions() const
{
    return expressions_;
}

// ----------------------------------------------------------------------------
void ExpressionList::accept(Visitor* visitor)
{
    visitor->visitExpressionList(this);
    for (const auto& expr : expressions_)
        expr->accept(visitor);
}
void ExpressionList::accept(ConstVisitor* visitor) const
{
    visitor->visitExpressionList(this);
    for (const auto& expr : expressions_)
        expr->accept(visitor);
}

// ----------------------------------------------------------------------------
void ExpressionList::swapChild(const Node* oldNode, Node* newNode)
{
    for (auto& expr : expressions_)
        if (expr == oldNode)
        {
            expr = dynamic_cast<Expression*>(newNode);
            newNode->setParent(this);
            return;
        }

    assert(false);
}

}
}
