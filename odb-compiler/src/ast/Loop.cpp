#include "odb-compiler/ast/Loop.hpp"
#include "odb-compiler/ast/Assignment.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/Identifier.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
Loop::Loop(SourceLocation* location) :
    Statement(location)
{
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
InfiniteLoop::InfiniteLoop(Block* body, SourceLocation* location) :
    Loop(location),
    body_(body)
{
}

// ----------------------------------------------------------------------------
InfiniteLoop::InfiniteLoop(SourceLocation* location) :
    Loop(location)
{
}

// ----------------------------------------------------------------------------
MaybeNull<Block> InfiniteLoop::body() const
{
    return body_.get();
}

// ----------------------------------------------------------------------------
std::string InfiniteLoop::toString() const
{
    return "InfiniteLoop";
}

// ----------------------------------------------------------------------------
void InfiniteLoop::accept(Visitor* visitor)
{
    visitor->visitInfiniteLoop(this);
}
void InfiniteLoop::accept(ConstVisitor* visitor) const
{
    visitor->visitInfiniteLoop(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange InfiniteLoop::children()
{
    if (body_)
    {
        return {body_};
    }
    else
    {
        return {};
    }
}

// ----------------------------------------------------------------------------
void InfiniteLoop::swapChild(const Node* oldNode, Node* newNode)
{
    if (body_ == oldNode)
        body_ = dynamic_cast<Block*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* InfiniteLoop::duplicateImpl() const
{
    return new InfiniteLoop(
        body_ ? body_->duplicate<Block>() : nullptr,
        location());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
WhileLoop::WhileLoop(Expression* continueCondition, Block* body, SourceLocation* location) :
    Loop(location),
    continueCondition_(continueCondition),
    body_(body)
{
}

// ----------------------------------------------------------------------------
WhileLoop::WhileLoop(Expression* continueCondition, SourceLocation* location) :
    Loop(location),
    continueCondition_(continueCondition)
{
}

// ----------------------------------------------------------------------------
Expression* WhileLoop::continueCondition() const
{
    return continueCondition_;
}

// ----------------------------------------------------------------------------
MaybeNull<Block> WhileLoop::body() const
{
    return body_.get();
}

// ----------------------------------------------------------------------------
std::string WhileLoop::toString() const
{
    return "WhileLoop";
}

// ----------------------------------------------------------------------------
void WhileLoop::accept(Visitor* visitor)
{
    visitor->visitWhileLoop(this);
}
void WhileLoop::accept(ConstVisitor* visitor) const
{
    visitor->visitWhileLoop(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange WhileLoop::children()
{
    if (body_)
    {
        return {continueCondition_, body_};
    }
    else
    {
        return {continueCondition_};
    }
}

// ----------------------------------------------------------------------------
void WhileLoop::swapChild(const Node* oldNode, Node* newNode)
{
    if (continueCondition_ == oldNode)
        continueCondition_ = dynamic_cast<Expression*>(newNode);
    else if (body_ == oldNode)
        body_ = dynamic_cast<Block*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* WhileLoop::duplicateImpl() const
{
    return new WhileLoop(
        continueCondition_->duplicate<Expression>(),
        body_ ? body_->duplicate<Block>() : nullptr,
        location());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
UntilLoop::UntilLoop(Expression* exitCondition, Block* body, SourceLocation* location) :
    Loop(location),
    exitCondition_(exitCondition),
    body_(body)
{
}

// ----------------------------------------------------------------------------
UntilLoop::UntilLoop(Expression* exitCondition, SourceLocation* location) :
    Loop(location),
    exitCondition_(exitCondition)
{
}

// ----------------------------------------------------------------------------
Expression* UntilLoop::exitCondition() const
{
    return exitCondition_;
}

// ----------------------------------------------------------------------------
MaybeNull<Block> UntilLoop::body() const
{
    return body_.get();
}

// ----------------------------------------------------------------------------
std::string UntilLoop::toString() const
{
    return "UntilLoop";
}

// ----------------------------------------------------------------------------
void UntilLoop::accept(Visitor* visitor)
{
    visitor->visitUntilLoop(this);
}
void UntilLoop::accept(ConstVisitor* visitor) const
{
    visitor->visitUntilLoop(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange UntilLoop::children()
{
    if (body_)
    {
        return {exitCondition_, body_};
    }
    else
    {
        return {exitCondition_};
    }
}

// ----------------------------------------------------------------------------
void UntilLoop::swapChild(const Node* oldNode, Node* newNode)
{
    if (exitCondition_ == oldNode)
        exitCondition_ = dynamic_cast<Expression*>(newNode);
    else if (body_ == oldNode)
        body_ = dynamic_cast<Block*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* UntilLoop::duplicateImpl() const
{
    return new UntilLoop(
        exitCondition_->duplicate<Expression>(),
        body_ ? body_->duplicate<Block>() : nullptr,
        location());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
ForLoop::ForLoop(Assignment* counter, Expression* endValue, Expression* stepValue, Identifier* nextIdentifier, Block* body, SourceLocation* location) :
    Loop(location),
    counter_(counter),
    endValue_(endValue),
    stepValue_(stepValue),
    nextIdentifier_(nextIdentifier),
    body_(body)
{
}

// ----------------------------------------------------------------------------
ForLoop::ForLoop(Assignment* counter, Expression* endValue, Expression* stepValue, Identifier* nextIdentifier, SourceLocation* location) :
    Loop(location),
    counter_(counter),
    endValue_(endValue),
    stepValue_(stepValue),
    nextIdentifier_(nextIdentifier)
{
}

// ----------------------------------------------------------------------------
ForLoop::ForLoop(Assignment* counter, Expression* endValue, Expression* stepValue, Block* body, SourceLocation* location) :
    Loop(location),
    counter_(counter),
    endValue_(endValue),
    stepValue_(stepValue),
    body_(body)
{
}

// ----------------------------------------------------------------------------
ForLoop::ForLoop(Assignment* counter, Expression* endValue, Expression* stepValue, SourceLocation* location) :
    Loop(location),
    counter_(counter),
    endValue_(endValue),
    stepValue_(stepValue)
{
}

// ----------------------------------------------------------------------------
ForLoop::ForLoop(Assignment* counter, Expression* endValue, Identifier* nextIdentifier, Block* body, SourceLocation* location) :
    Loop(location),
    counter_(counter),
    endValue_(endValue),
    nextIdentifier_(nextIdentifier),
    body_(body)
{
}

// ----------------------------------------------------------------------------
ForLoop::ForLoop(Assignment* counter, Expression* endValue, Identifier* nextIdentifier, SourceLocation* location) :
    Loop(location),
    counter_(counter),
    endValue_(endValue),
    nextIdentifier_(nextIdentifier)
{
}

// ----------------------------------------------------------------------------
ForLoop::ForLoop(Assignment* counter, Expression* endValue, Block* body, SourceLocation* location) :
    Loop(location),
    counter_(counter),
    endValue_(endValue),
    body_(body)
{
}

// ----------------------------------------------------------------------------
ForLoop::ForLoop(Assignment* counter, Expression* endValue, SourceLocation* location) :
    Loop(location),
    counter_(counter),
    endValue_(endValue)
{
}

// ----------------------------------------------------------------------------
Assignment* ForLoop::counter() const
{
    return counter_;
}

// ----------------------------------------------------------------------------
Expression* ForLoop::endValue() const
{
    return endValue_;
}

// ----------------------------------------------------------------------------
MaybeNull<Expression> ForLoop::stepValue() const
{
    return stepValue_.get();
}

// ----------------------------------------------------------------------------
MaybeNull<Identifier> ForLoop::nextIdentifier() const
{
    return nextIdentifier_.get();
}

// ----------------------------------------------------------------------------
MaybeNull<Block> ForLoop::body() const
{
    return body_.get();
}

// ----------------------------------------------------------------------------
std::string ForLoop::toString() const
{
    return "ForLoop";
}

// ----------------------------------------------------------------------------
void ForLoop::accept(Visitor* visitor)
{
    visitor->visitForLoop(this);
}
void ForLoop::accept(ConstVisitor* visitor) const
{
    visitor->visitForLoop(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange ForLoop::children()
{
    ChildRange children;
    children.push_back(counter_);
    children.push_back(endValue_);
    if (stepValue_)
    {
        children.push_back(stepValue_);
    }
    if (nextIdentifier_)
    {
        children.push_back(nextIdentifier_);
    }
    if (body_)
    {
        children.push_back(body_);
    }
    return children;
}

// ----------------------------------------------------------------------------
void ForLoop::swapChild(const Node* oldNode, Node* newNode)
{
    if (counter_ == oldNode)
        counter_ = dynamic_cast<Assignment*>(newNode);
    else if (endValue_ == oldNode)
        endValue_ = dynamic_cast<Expression*>(newNode);
    else if (stepValue_ == oldNode)
        stepValue_ = dynamic_cast<Expression*>(newNode);
    else if (nextIdentifier_ == oldNode)
        nextIdentifier_ = dynamic_cast<Identifier*>(newNode);
    else if (body_ == oldNode)
        body_ = dynamic_cast<Block*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* ForLoop::duplicateImpl() const
{
    return new ForLoop(
        counter_->duplicate<Assignment>(),
        endValue_->duplicate<Expression>(),
        stepValue_ ? stepValue_->duplicate<Expression>() : nullptr,
        nextIdentifier_ ? nextIdentifier_->duplicate<Identifier>() : nullptr,
        body_ ? body_->duplicate<Block>() : nullptr,
        location());
}

}
