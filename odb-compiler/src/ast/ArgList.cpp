#include "odb-compiler/ast/ArgList.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
ArgList::ArgList(SourceLocation* location) :
    Node(location)
{
}

// ----------------------------------------------------------------------------
ArgList::ArgList(Expression* expr, SourceLocation* location) :
    Node(location)
{
    appendExpression(expr);
}

// ----------------------------------------------------------------------------
void ArgList::appendExpression(Expression* expr)
{
    expr->setParent(this);
    expressions_.push_back(expr);

    location()->unionize(expr->location());
}

// ----------------------------------------------------------------------------
const std::vector<Reference<Expression>>& ArgList::expressions() const
{
    return expressions_;
}

// ----------------------------------------------------------------------------
std::string ArgList::toString() const
{
    return "ArgList(" + std::to_string(expressions_.size()) + ")";
}

// ----------------------------------------------------------------------------
void ArgList::accept(Visitor* visitor)
{
    visitor->visitArgList(this);
    for (const auto& expr : expressions_)
        expr->accept(visitor);
}
void ArgList::accept(ConstVisitor* visitor) const
{
    visitor->visitArgList(this);
    for (const auto& expr : expressions_)
        expr->accept(visitor);
}

// ----------------------------------------------------------------------------
void ArgList::swapChild(const Node* oldNode, Node* newNode)
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
Node* ArgList::duplicateImpl() const
{
    ArgList* el = new ArgList(location());
    for (const auto& expr : expressions_)
        el->appendExpression(expr->duplicate<Expression>());
    return el;
}

}
