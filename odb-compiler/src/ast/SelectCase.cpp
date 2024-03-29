#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/SelectCase.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
Select::Select(Program* program, SourceLocation* location, Expression* expr, CaseList* cases, SourceLocation* beginSelect, SourceLocation* endSelect)
    : Statement(program, location)
    , expr_(expr)
    , cases_(cases)
    , beginLoc_(beginSelect)
    , endLoc_(endSelect)
{
}

// ----------------------------------------------------------------------------
Select::Select(Program* program, SourceLocation* location, Expression* expr, SourceLocation* beginSelect, SourceLocation* endSelect)
    : Statement(program, location)
    , expr_(expr)
    , beginLoc_(beginSelect)
    , endLoc_(endSelect)
{
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
}
void Select::accept(ConstVisitor* visitor) const
{
    visitor->visitSelect(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange Select::children()
{
    if (cases_)
    {
        return {expr_, cases_};
    }
    else
    {
        return {expr_};
    }
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
}

// ----------------------------------------------------------------------------
Node* Select::duplicateImpl() const
{
    return new Select(
        program(),
        location(),
        expr_->duplicate<Expression>(),
        cases_ ? cases_->duplicate<CaseList>() : nullptr,
        beginSelectLocation(),
        endSelectLocation());
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
CaseList::CaseList(Program* program, SourceLocation* location, Case* case_)
    : Node(program, location)
{
    appendCase(case_);
}

// ----------------------------------------------------------------------------
CaseList::CaseList(Program* program, SourceLocation* location, DefaultCase* case_)
    : Node(program, location)
{
    appendDefaultCase(case_);
}

// ----------------------------------------------------------------------------
CaseList::CaseList(Program* program, SourceLocation* location)
    : Node(program, location)
{
}

// ----------------------------------------------------------------------------
void CaseList::appendCase(Case* case_)
{
    cases_.emplace_back(case_);

    location()->unionize(case_->location());
}

// ----------------------------------------------------------------------------
void CaseList::appendDefaultCase(DefaultCase* case_)
{
    defaults_.emplace_back(case_);

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
    return !defaults_.empty() ? defaults_.back().get() : nullptr;
}

// ----------------------------------------------------------------------------
void CaseList::accept(Visitor* visitor)
{
    visitor->visitCaseList(this);
}
void CaseList::accept(ConstVisitor* visitor) const
{
    visitor->visitCaseList(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange CaseList::children()
{
    ChildRange children;
    for (const auto& case_ : cases_)
    {
        children.push_back(case_);
    }
    for (const auto& default_ : defaults_)
    {
        children.push_back(default_);
    }
    return children;
}

// ----------------------------------------------------------------------------
void CaseList::swapChild(const Node* oldNode, Node* newNode)
{
    for (auto& case_ : cases_)
        if (case_ == oldNode)
        {
            case_ = dynamic_cast<Case*>(newNode);
            return;
        }
    for (auto& default_ : defaults_)
        if (default_ == oldNode)
        {
            default_ = dynamic_cast<DefaultCase*>(newNode);
            return;
        }

    assert(false);
}

// ----------------------------------------------------------------------------
Node* CaseList::duplicateImpl() const
{
    CaseList* cl = new CaseList(program(), location());
    for (const auto& case_ : cases_)
        cl->appendCase(case_->duplicate<Case>());
    for (const auto& default_ : defaults_)
        cl->appendDefaultCase(default_);
    return cl;
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
Case::Case(Program* program, SourceLocation* location, Expression* expr, Block* body)
    : Node(program, location)
    , expr_(expr)
    , body_(body)
{
}

// ----------------------------------------------------------------------------
Case::Case(Program* program, SourceLocation* location, Expression* expr)
    : Node(program, location)
    , expr_(expr)
{
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
}
void Case::accept(ConstVisitor* visitor) const
{
    visitor->visitCase(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange Case::children()
{
    if (body_)
    {
        return {expr_, body_};
    }
    else
    {
        return {expr_};
    }
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
}

// ----------------------------------------------------------------------------
Node* Case::duplicateImpl() const
{
    return new Case(
        program(),
        location(),
        expr_->duplicate<Expression>(),
        body_ ? body_->duplicate<Block>() : nullptr);
}

// ============================================================================
// ============================================================================

// ----------------------------------------------------------------------------
DefaultCase::DefaultCase(Program* program, SourceLocation* location, Block* body, SourceLocation* beginCaseLoc, SourceLocation* endCaseLoc)
    : Node(program, location)
    , body_(body)
    , beginLoc_(beginCaseLoc)
    , endLoc_(endCaseLoc)
{
}

// ----------------------------------------------------------------------------
DefaultCase::DefaultCase(Program* program, SourceLocation* location, SourceLocation* beginCaseLoc, SourceLocation* endCaseLoc)
    : Node(program, location)
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
}
void DefaultCase::accept(ConstVisitor* visitor) const
{
    visitor->visitDefaultCase(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange DefaultCase::children()
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
void DefaultCase::swapChild(const Node* oldNode, Node* newNode)
{
    if (body_ == oldNode)
        body_ = dynamic_cast<Block*>(newNode);
    else
        assert(false);
}

// ----------------------------------------------------------------------------
Node* DefaultCase::duplicateImpl() const
{
    return new DefaultCase(
        program(),
        location(),
        body_ ? body_->duplicate<Block>() : nullptr,
        beginCaseLocation(),
        endCaseLocation());
}

}
