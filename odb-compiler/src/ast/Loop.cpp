#include "odb-compiler/ast/Assignment.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/Loop.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/AnnotatedSymbol.hpp"
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
    body->setParent(this);
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

    newNode->setParent(this);
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
    continueCondition->setParent(this);
    body->setParent(this);
}

// ----------------------------------------------------------------------------
WhileLoop::WhileLoop(Expression* continueCondition, SourceLocation* location) :
    Loop(location),
    continueCondition_(continueCondition)
{
    continueCondition->setParent(this);
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

    newNode->setParent(this);
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
    exitCondition->setParent(this);
    body->setParent(this);
}

// ----------------------------------------------------------------------------
UntilLoop::UntilLoop(Expression* exitCondition, SourceLocation* location) :
    Loop(location),
    exitCondition_(exitCondition)
{
    exitCondition->setParent(this);
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

    newNode->setParent(this);
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
ForLoop::ForLoop(Assignment* counter, Expression* endValue, Expression* stepValue, AnnotatedSymbol* nextSymbol, Block* body, SourceLocation* location) :
    Loop(location),
    counter_(counter),
    endValue_(endValue),
    stepValue_(stepValue),
    nextSymbol_(nextSymbol),
    body_(body)
{
    counter->setParent(this);
    endValue->setParent(this);
    stepValue->setParent(this);
    nextSymbol->setParent(this);
    body->setParent(this);
}

// ----------------------------------------------------------------------------
ForLoop::ForLoop(Assignment* counter, Expression* endValue, Expression* stepValue, AnnotatedSymbol* nextSymbol, SourceLocation* location) :
    Loop(location),
    counter_(counter),
    endValue_(endValue),
    stepValue_(stepValue),
    nextSymbol_(nextSymbol)
{
    counter->setParent(this);
    endValue->setParent(this);
    stepValue->setParent(this);
    nextSymbol->setParent(this);
}

// ----------------------------------------------------------------------------
ForLoop::ForLoop(Assignment* counter, Expression* endValue, Expression* stepValue, Block* body, SourceLocation* location) :
    Loop(location),
    counter_(counter),
    endValue_(endValue),
    stepValue_(stepValue),
    body_(body)
{
    counter->setParent(this);
    endValue->setParent(this);
    stepValue->setParent(this);
    body->setParent(this);
}

// ----------------------------------------------------------------------------
ForLoop::ForLoop(Assignment* counter, Expression* endValue, Expression* stepValue, SourceLocation* location) :
    Loop(location),
    counter_(counter),
    endValue_(endValue),
    stepValue_(stepValue)
{
    counter->setParent(this);
    endValue->setParent(this);
    stepValue->setParent(this);
}

// ----------------------------------------------------------------------------
ForLoop::ForLoop(Assignment* counter, Expression* endValue, AnnotatedSymbol* nextSymbol, Block* body, SourceLocation* location) :
    Loop(location),
    counter_(counter),
    endValue_(endValue),
    nextSymbol_(nextSymbol),
    body_(body)
{
    counter->setParent(this);
    endValue->setParent(this);
    nextSymbol->setParent(this);
    body->setParent(this);
}

// ----------------------------------------------------------------------------
ForLoop::ForLoop(Assignment* counter, Expression* endValue, AnnotatedSymbol* nextSymbol, SourceLocation* location) :
    Loop(location),
    counter_(counter),
    endValue_(endValue),
    nextSymbol_(nextSymbol)
{
    counter->setParent(this);
    endValue->setParent(this);
    nextSymbol->setParent(this);
}

// ----------------------------------------------------------------------------
ForLoop::ForLoop(Assignment* counter, Expression* endValue, Block* body, SourceLocation* location) :
    Loop(location),
    counter_(counter),
    endValue_(endValue),
    body_(body)
{
    counter->setParent(this);
    endValue->setParent(this);
    body->setParent(this);
}

// ----------------------------------------------------------------------------
ForLoop::ForLoop(Assignment* counter, Expression* endValue, SourceLocation* location) :
    Loop(location),
    counter_(counter),
    endValue_(endValue)
{
    counter->setParent(this);
    endValue->setParent(this);
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
MaybeNull<AnnotatedSymbol> ForLoop::nextSymbol() const
{
    return nextSymbol_.get();
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
    if (nextSymbol_)
    {
        children.push_back(nextSymbol_);
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
    else if (nextSymbol_ == oldNode)
        nextSymbol_ = dynamic_cast<AnnotatedSymbol*>(newNode);
    else if (body_ == oldNode)
        body_ = dynamic_cast<Block*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* ForLoop::duplicateImpl() const
{
    return new ForLoop(
        counter_->duplicate<Assignment>(),
        endValue_->duplicate<Expression>(),
        stepValue_ ? stepValue_->duplicate<Expression>() : nullptr,
        nextSymbol_ ? nextSymbol_->duplicate<AnnotatedSymbol>() : nullptr,
        body_ ? body_->duplicate<Block>() : nullptr,
        location());
}

}
