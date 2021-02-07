#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/Conditional.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
Conditional::Conditional(Expression* condition, Block* trueBranch, Block* falseBranch, SourceLocation* location) :
    Statement(location),
    cond_(condition),
    true_(trueBranch),
    false_(falseBranch)
{
    condition->setParent(this);
    if (trueBranch)
        trueBranch->setParent(this);
    if (falseBranch)
        falseBranch->setParent(this);
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
void Conditional::accept(Visitor* visitor)
{
    visitor->visitConditional(this);
    cond_->accept(visitor);
    if (true_)
        true_->accept(visitor);
    if (false_)
        false_->accept(visitor);
}
void Conditional::accept(ConstVisitor* visitor) const
{
    visitor->visitConditional(this);
    cond_->accept(visitor);
    if (true_)
        true_->accept(visitor);
    if (false_)
        false_->accept(visitor);
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

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* Conditional::duplicateImpl() const
{
    return new Conditional(
        cond_->duplicate<Expression>(),
        true_ ? true_->duplicate<Block>() : nullptr,
        false_ ? false_->duplicate<Block>() : nullptr,
        location());
}

}
