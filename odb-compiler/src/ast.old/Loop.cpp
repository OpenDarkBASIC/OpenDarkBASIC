#include "odb-compiler/ast/Loop.hpp"
#include "odb-compiler/ast/Assignment.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/Identifier.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
Loop::Loop(Program* program, SourceLocation* location) :
    Statement(program, location)
{
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
InfiniteLoop::InfiniteLoop(Program* program, SourceLocation* location, Block* body) :
    Loop(program, location),
    body_(body)
{
}

// ----------------------------------------------------------------------------
InfiniteLoop::InfiniteLoop(Program* program, SourceLocation* location) :
    Loop(program, location)
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
        program(),
        location(),
        body_ ? body_->duplicate<Block>() : nullptr);
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
WhileLoop::WhileLoop(Program* program, SourceLocation* location, Expression* continueCondition, Block* body) :
    Loop(program, location),
    continueCondition_(continueCondition),
    body_(body)
{
}

// ----------------------------------------------------------------------------
WhileLoop::WhileLoop(Program* program, SourceLocation* location, Expression* continueCondition) :
    Loop(program, location),
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
        program(),
        location(),
        continueCondition_->duplicate<Expression>(),
        body_ ? body_->duplicate<Block>() : nullptr);
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
UntilLoop::UntilLoop(Program* program, SourceLocation* location, Expression* exitCondition, Block* body) :
    Loop(program, location),
    exitCondition_(exitCondition),
    body_(body)
{
}

// ----------------------------------------------------------------------------
UntilLoop::UntilLoop(Program* program, SourceLocation* location, Expression* exitCondition) :
    Loop(program, location),
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
        program(),
        location(),
        exitCondition_->duplicate<Expression>(),
        body_ ? body_->duplicate<Block>() : nullptr);
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
ForLoop::ForLoop(Program* program, SourceLocation* location, Assignment* counter, Expression* endValue, Expression* stepValue, Identifier* nextIdentifier, Block* body) :
    Loop(program, location),
    counter_(counter),
    endValue_(endValue),
    stepValue_(stepValue),
    nextIdentifier_(nextIdentifier),
    body_(body)
{
}

// ----------------------------------------------------------------------------
ForLoop::ForLoop(Program* program, SourceLocation* location, Assignment* counter, Expression* endValue, Expression* stepValue, Identifier* nextIdentifier) :
    Loop(program, location),
    counter_(counter),
    endValue_(endValue),
    stepValue_(stepValue),
    nextIdentifier_(nextIdentifier)
{
}

// ----------------------------------------------------------------------------
ForLoop::ForLoop(Program* program, SourceLocation* location, Assignment* counter, Expression* endValue, Expression* stepValue, Block* body) :
    Loop(program, location),
    counter_(counter),
    endValue_(endValue),
    stepValue_(stepValue),
    body_(body)
{
}

// ----------------------------------------------------------------------------
ForLoop::ForLoop(Program* program, SourceLocation* location, Assignment* counter, Expression* endValue, Expression* stepValue) :
    Loop(program, location),
    counter_(counter),
    endValue_(endValue),
    stepValue_(stepValue)
{
}

// ----------------------------------------------------------------------------
ForLoop::ForLoop(Program* program, SourceLocation* location, Assignment* counter, Expression* endValue, Identifier* nextIdentifier, Block* body) :
    Loop(program, location),
    counter_(counter),
    endValue_(endValue),
    stepValue_(new ast::ByteLiteral(program, endValue->location(), 1)),
    nextIdentifier_(nextIdentifier),
    body_(body)
{
}

// ----------------------------------------------------------------------------
ForLoop::ForLoop(Program* program, SourceLocation* location, Assignment* counter, Expression* endValue, Identifier* nextIdentifier) :
    Loop(program, location),
    counter_(counter),
    endValue_(endValue),
    stepValue_(new ast::ByteLiteral(program, endValue->location(), 1)),
    nextIdentifier_(nextIdentifier)
{
}

// ----------------------------------------------------------------------------
ForLoop::ForLoop(Program* program, SourceLocation* location, Assignment* counter, Expression* endValue, Block* body) :
    Loop(program, location),
    counter_(counter),
    endValue_(endValue),
    stepValue_(new ast::ByteLiteral(program, endValue->location(), 1)),
    body_(body)
{
}

// ----------------------------------------------------------------------------
ForLoop::ForLoop(Program* program, SourceLocation* location, Assignment* counter, Expression* endValue) :
    Loop(program, location),
    counter_(counter),
    endValue_(endValue),
    stepValue_(new ast::ByteLiteral(program, endValue->location(), 1))
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
Expression* ForLoop::stepValue() const
{
    return stepValue_;
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
        program(),
        location(),
        counter_->duplicate<Assignment>(),
        endValue_->duplicate<Expression>(),
        stepValue_ ? stepValue_->duplicate<Expression>() : nullptr,
        nextIdentifier_ ? nextIdentifier_->duplicate<Identifier>() : nullptr,
        body_ ? body_->duplicate<Block>() : nullptr);
}

}
