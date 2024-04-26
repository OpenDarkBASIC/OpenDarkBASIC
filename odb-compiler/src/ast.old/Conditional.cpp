#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/Conditional.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
Conditional::Conditional(Program* program, SourceLocation* location, Expression* condition, Block* trueBranch, Block* falseBranch) :
    Statement(program, location),
    cond_(condition),
    true_(trueBranch),
    false_(falseBranch)
{
}

// ----------------------------------------------------------------------------
Expression* Conditional::condition() const
{
    return cond_;
}

// ----------------------------------------------------------------------------
MaybeNull<Block> Conditional::trueBranch() const
{
    return true_.get();
}

// ----------------------------------------------------------------------------
MaybeNull<Block> Conditional::falseBranch() const
{
    return false_.get();
}

// ----------------------------------------------------------------------------
std::string Conditional::toString() const
{
    return "Conditional";
}

// ----------------------------------------------------------------------------
void Conditional::accept(Visitor* visitor)
{
    visitor->visitConditional(this);
}
void Conditional::accept(ConstVisitor* visitor) const
{
    visitor->visitConditional(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange Conditional::children()
{
    ChildRange children;
    children.push_back(cond_);
    if (true_)
    {
        children.push_back(true_);
    }
    if (false_)
    {
        children.push_back(false_);
    }
    return children;
}

// ----------------------------------------------------------------------------
void Conditional::swapChild(const Node* oldNode, Node* newNode)
{
    if (cond_ == oldNode)
        cond_ = dynamic_cast<Expression*>(newNode);
    else if (true_ == oldNode)
        true_ = dynamic_cast<Block*>(newNode);
    else if (false_ == oldNode)
        false_ = dynamic_cast<Block*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* Conditional::duplicateImpl() const
{
    return new Conditional(
        program(),
        location(),
        cond_->duplicate<Expression>(),
        true_ ? true_->duplicate<Block>() : nullptr,
        false_ ? false_->duplicate<Block>() : nullptr);
}

}
