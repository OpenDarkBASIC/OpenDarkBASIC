#include "odb-compiler/ast/ArgList.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
ArgList::ArgList(Program* program, SourceLocation* location) :
    Node(program, location)
{
}

// ----------------------------------------------------------------------------
ArgList::ArgList(Program* program, SourceLocation* location, Expression* expr) :
    Node(program, location)
{
    appendExpression(expr);
}

// ----------------------------------------------------------------------------
void ArgList::appendExpression(Expression* expr)
{
    expressions_.emplace_back(expr);

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
}
void ArgList::accept(ConstVisitor* visitor) const
{
    visitor->visitArgList(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange ArgList::children()
{
    ChildRange children;
    for (Expression* e : expressions_)
    {
        children.push_back(e);
    }
    return children;
}

// ----------------------------------------------------------------------------
void ArgList::swapChild(const Node* oldNode, Node* newNode)
{
    for (auto& expr : expressions_)
        if (expr == oldNode)
        {
            expr = dynamic_cast<Expression*>(newNode);
            return;
        }

    assert(false);
}

// ----------------------------------------------------------------------------
Node* ArgList::duplicateImpl() const
{
    ArgList* el = new ArgList(program(), location());
    for (const auto& expr : expressions_)
        el->appendExpression(expr->duplicate<Expression>());
    return el;
}

}
