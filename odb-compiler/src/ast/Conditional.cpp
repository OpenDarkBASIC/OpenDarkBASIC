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
void Conditional::accept(Visitor* visitor) const
{
    visitor->visitConditional(this);
    cond_->accept(visitor);
    if (true_)
        true_->accept(visitor);
    if (false_)
        false_->accept(visitor);
}

}
