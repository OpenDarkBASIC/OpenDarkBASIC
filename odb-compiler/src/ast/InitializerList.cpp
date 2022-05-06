#include "odb-compiler/ast/InitializerList.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
InitializerList::InitializerList(SourceLocation* location) :
    Expression(location),
    typeBeingInitialized_(Type::getUnknown())
{
}

// ----------------------------------------------------------------------------
InitializerList::InitializerList(Expression* expr, SourceLocation* location) :
    Expression(location),
    typeBeingInitialized_(Type::getUnknown())
{
    appendExpression(expr);
}

// ----------------------------------------------------------------------------
void InitializerList::appendExpression(Expression* expr)
{
    expressions_.emplace_back(expr);

    location()->unionize(expr->location());
}

// ----------------------------------------------------------------------------
const std::vector<Reference<Expression>>& InitializerList::expressions() const
{
    return expressions_;
}

void InitializerList::setTypeBeingInitialized(Type typeBeingInitialized)
{
    typeBeingInitialized_ = typeBeingInitialized;
}

Type InitializerList::getType() const
{
    return typeBeingInitialized_;
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
}
void InitializerList::accept(ConstVisitor* visitor) const
{
    visitor->visitInitializerList(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange InitializerList::children()
{
    ChildRange children;
    for (const auto& expr : expressions_)
    {
        children.push_back(expr);
    }
    return children;
}

// ----------------------------------------------------------------------------
void InitializerList::swapChild(const Node* oldNode, Node* newNode)
{
    if (oldNode == newNode)
        return;

    for (auto& expr : expressions_)
        if (expr == oldNode)
        {
            expr = dynamic_cast<Expression*>(newNode);
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

