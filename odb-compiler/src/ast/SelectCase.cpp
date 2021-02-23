#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/SelectCase.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
Select::Select(Expression* expr, CaseList* cases, SourceLocation* location, SourceLocation* beginSelect, SourceLocation* endSelect)
    : Statement(location)
    , expr_(expr)
    , cases_(cases)
    , beginLoc_(beginSelect)
    , endLoc_(endSelect)
{
    expr->setParent(this);
    cases->setParent(this);
}

// ----------------------------------------------------------------------------
Select::Select(Expression* expr, SourceLocation* location, SourceLocation* beginSelect, SourceLocation* endSelect)
    : Statement(location)
    , expr_(expr)
    , beginLoc_(beginSelect)
    , endLoc_(endSelect)
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
SourceLocation* Select::beginSelectLocation() const
{
    return beginLoc_;
}

// ----------------------------------------------------------------------------
SourceLocation* Select::endSelectLocation() const
{
    return endLoc_;
}

// ----------------------------------------------------------------------------
std::string Select::toString() const
{
    return "Select";
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
        location(),
        beginSelectLocation(),
        endSelectLocation());
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
{
    appendDefaultCase(case_);
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
void CaseList::appendDefaultCase(DefaultCase* case_)
{
    defaults_.push_back(case_);
    case_->setParent(this);

    location()->unionize(case_->location());
}

// ----------------------------------------------------------------------------
std::string CaseList::toString() const
{
    return "CaseList(" + std::to_string(cases_.size()) + ")";
}

// ----------------------------------------------------------------------------
const std::vector<Reference<Case>>& CaseList::cases() const
{
    return cases_;
}

// ----------------------------------------------------------------------------
const std::vector<Reference<DefaultCase>>& CaseList::defaultCases() const
{
    return defaults_;
}

// ----------------------------------------------------------------------------
MaybeNull<DefaultCase> CaseList::defaultCase() const
{
    return defaults_.size() > 0 ? defaults_.back().get() : nullptr;
}

// ----------------------------------------------------------------------------
void CaseList::accept(Visitor* visitor)
{
    visitor->visitCaseList(this);
    for (const auto& case_ : cases_)
        case_->accept(visitor);
    for (const auto& default_ : defaults_)
        default_->accept(visitor);
}
void CaseList::accept(ConstVisitor* visitor) const
{
    visitor->visitCaseList(this);
    for (const auto& case_ : cases_)
        case_->accept(visitor);
    for (const auto& default_ : defaults_)
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
    for (auto& default_ : defaults_)
        if (default_ == oldNode)
        {
            default_ = dynamic_cast<DefaultCase*>(newNode);
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
    for (const auto& default_ : defaults_)
        cl->appendDefaultCase(default_);
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
std::string Case::toString() const
{
    return "Case";
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
DefaultCase::DefaultCase(Block* body, SourceLocation* location, SourceLocation* beginCaseLoc, SourceLocation* endCaseLoc)
    : Node(location)
    , body_(body)
    , beginLoc_(beginCaseLoc)
    , endLoc_(endCaseLoc)
{
    body->setParent(this);
}

// ----------------------------------------------------------------------------
DefaultCase::DefaultCase(SourceLocation* location, SourceLocation* beginCaseLoc, SourceLocation* endCaseLoc)
    : Node(location)
    , beginLoc_(beginCaseLoc)
    , endLoc_(endCaseLoc)
{
}

// ----------------------------------------------------------------------------
MaybeNull<Block> DefaultCase::body() const
{
    return body_.get();
}

// ----------------------------------------------------------------------------
SourceLocation* DefaultCase::beginCaseLocation() const
{
    return beginLoc_;
}

// ----------------------------------------------------------------------------
SourceLocation* DefaultCase::endCaseLocation() const
{
    return endLoc_;
}

// ----------------------------------------------------------------------------
std::string DefaultCase::toString() const
{
    return "DefaultCase";
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
        location(),
        beginCaseLocation(),
        endCaseLocation());
}

}
