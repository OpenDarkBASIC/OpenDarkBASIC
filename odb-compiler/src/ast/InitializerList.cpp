#include "odb-compiler/ast/InitializerList.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
InitializerList::InitializerList(SourceLocation* location) :
    Expression(location)
{
}

// ----------------------------------------------------------------------------
InitializerList::InitializerList(Expression* expr, SourceLocation* location) :
    Expression(location)
{
    appendExpression(expr);
}

// ----------------------------------------------------------------------------
void InitializerList::appendExpression(Expression* expr)
{
    expr->setParent(this);
    expressions_.push_back(expr);

    location()->unionize(expr->location());
}

// ----------------------------------------------------------------------------
const std::vector<Reference<Expression>>& InitializerList::expressions() const
{
    return expressions_;
}

// ----------------------------------------------------------------------------
std::string InitializerList::toString() const
{
    return "InitializerList(" + std::to_string(expressions_.size()) + ")";
}

// ----------------------------------------------------------------------------
void InitializerList::accept(Visitor* visitor)
{
    visitor->visitInitializerList(this);
    for (const auto& expr : expressions_)
        expr->accept(visitor);
}
void InitializerList::accept(ConstVisitor* visitor) const
{
    visitor->visitInitializerList(this);
    for (const auto& expr : expressions_)
        expr->accept(visitor);
}

// ----------------------------------------------------------------------------
void InitializerList::swapChild(const Node* oldNode, Node* newNode)
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

// ----------------------------------------------------------------------------
Node* InitializerList::duplicateImpl() const
{
    InitializerList* el = new InitializerList(location());
    for (const auto& expr : expressions_)
        el->appendExpression(expr->duplicate<Expression>());
    return el;
}

}

