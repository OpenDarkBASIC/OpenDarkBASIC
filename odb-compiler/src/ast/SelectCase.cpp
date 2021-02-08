#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/SelectCase.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
Select::Select(Expression* expr, CaseList* cases, SourceLocation* location)
    : Statement(location)
    , expr_(expr)
    , cases_(cases)
{
    expr->setParent(this);
    cases->setParent(this);
}

// ----------------------------------------------------------------------------
Select::Select(Expression* expr, SourceLocation* location)
    : Statement(location)
    , expr_(expr)
{
    expr->setParent(this);
}

// ----------------------------------------------------------------------------
Expression* Select::expression() const
{
    return expr_;
}

// ----------------------------------------------------------------------------
MaybeNull<CaseList> Select::cases() const
{
    return cases_.get();
}

// ----------------------------------------------------------------------------
void Select::accept(Visitor* visitor)
{
    visitor->visitSelect(this);
    expr_->accept(visitor);
    if (cases_)
        cases_->accept(visitor);
}
void Select::accept(ConstVisitor* visitor) const
{
    visitor->visitSelect(this);
    expr_->accept(visitor);
    if (cases_)
        cases_->accept(visitor);
}

// ----------------------------------------------------------------------------
void Select::swapChild(const Node* oldNode, Node* newNode)
{
    if (expr_ == oldNode)
        expr_ = dynamic_cast<Expression*>(newNode);
    else if (cases_ == oldNode)
        cases_ = dynamic_cast<CaseList*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* Select::duplicateImpl() const
{
    return new Select(
        expr_->duplicate<Expression>(),
        cases_ ? cases_->duplicate<CaseList>() : nullptr,
        location());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
CaseList::CaseList(Case* case_, SourceLocation* location)
    : Node(location)
{
    appendCase(case_);
}

// ----------------------------------------------------------------------------
CaseList::CaseList(DefaultCase* case_, SourceLocation* location)
    : Node(location)
    , default_(case_)
{
    case_->setParent(this);
}

// ----------------------------------------------------------------------------
CaseList::CaseList(SourceLocation* location)
    : Node(location)
{
}

// ----------------------------------------------------------------------------
void CaseList::appendCase(Case* case_)
{
    cases_.push_back(case_);
    case_->setParent(this);

    location()->unionize(case_->location());
}

// ----------------------------------------------------------------------------
void CaseList::setDefaultCase(DefaultCase* case_)
{
    default_ = case_;
    case_->setParent(this);
}

// ----------------------------------------------------------------------------
const std::vector<Reference<Case>>& CaseList::cases() const
{
    return cases_;
}

// ----------------------------------------------------------------------------
MaybeNull<DefaultCase> CaseList::defaultCase() const
{
    return default_.get();
}

// ----------------------------------------------------------------------------
void CaseList::accept(Visitor* visitor)
{
    visitor->visitCaseList(this);
    for (const auto& case_ : cases_)
        case_->accept(visitor);
    if (default_)
        default_->accept(visitor);
}
void CaseList::accept(ConstVisitor* visitor) const
{
    visitor->visitCaseList(this);
    for (const auto& case_ : cases_)
        case_->accept(visitor);
    if (default_)
        default_->accept(visitor);
}

// ----------------------------------------------------------------------------
void CaseList::swapChild(const Node* oldNode, Node* newNode)
{
    for (auto& case_ : cases_)
        if (case_ == oldNode)
        {
            case_ = dynamic_cast<Case*>(newNode);
            newNode->setParent(this);
            return;
        }

    assert(false);
}

// ----------------------------------------------------------------------------
Node* CaseList::duplicateImpl() const
{
    CaseList* cl = new CaseList(location());
    for (const auto& case_ : cases_)
        cl->appendCase(case_->duplicate<Case>());
    return cl;
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
Case::Case(Expression* expr, Block* body, SourceLocation* location)
    : Node(location)
    , expr_(expr)
    , body_(body)
{
    expr->setParent(this);
    body->setParent(this);
}

// ----------------------------------------------------------------------------
Case::Case(Expression* expr, SourceLocation* location)
    : Node(location)
    , expr_(expr)
{
    expr->setParent(this);
}

// ----------------------------------------------------------------------------
Expression* Case::expression() const
{
    return expr_;
}

// ----------------------------------------------------------------------------
MaybeNull<Block> Case::body() const
{
    return body_.get();
}

// ----------------------------------------------------------------------------
void Case::accept(Visitor* visitor)
{
    visitor->visitCase(this);
    expr_->accept(visitor);
    if (body_)
        body_->accept(visitor);
}
void Case::accept(ConstVisitor* visitor) const
{
    visitor->visitCase(this);
    expr_->accept(visitor);
    if (body_)
        body_->accept(visitor);
}

// ----------------------------------------------------------------------------
void Case::swapChild(const Node* oldNode, Node* newNode)
{
    if (expr_ == oldNode)
        expr_ = dynamic_cast<Expression*>(newNode);
    else if (body_ == oldNode)
        body_ = dynamic_cast<Block*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* Case::duplicateImpl() const
{
    return new Case(
        expr_->duplicate<Expression>(),
        body_ ? body_->duplicate<Block>() : nullptr,
        location());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
DefaultCase::DefaultCase(Block* body, SourceLocation* location)
    : Node(location)
    , body_(body)
{
    body->setParent(this);
}

// ----------------------------------------------------------------------------
DefaultCase::DefaultCase(SourceLocation* location)
    : Node(location)
{
}

// ----------------------------------------------------------------------------
MaybeNull<Block> DefaultCase::body() const
{
    return body_.get();
}

// ----------------------------------------------------------------------------
void DefaultCase::accept(Visitor* visitor)
{
    visitor->visitDefaultCase(this);
    if (body_)
        body_->accept(visitor);
}
void DefaultCase::accept(ConstVisitor* visitor) const
{
    visitor->visitDefaultCase(this);
    if (body_)
        body_->accept(visitor);
}

// ----------------------------------------------------------------------------
void DefaultCase::swapChild(const Node* oldNode, Node* newNode)
{
    if (body_ == oldNode)
        body_ = dynamic_cast<Block*>(newNode);
    else
        assert(false);

    newNode->setParent(this);
}

// ----------------------------------------------------------------------------
Node* DefaultCase::duplicateImpl() const
{
    return new DefaultCase(
        body_ ? body_->duplicate<Block>() : nullptr,
        location());
}

}
