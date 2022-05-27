#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-sdk/MaybeNull.hpp"
#include <vector>

namespace odb::ast {

class Block;
class Case;
class CaseList;
class DefaultCase;
class Expression;

class ODBCOMPILER_PUBLIC_API Select final : public Statement
{
public:
    Select(Program* program, SourceLocation* location, Expression* expr, CaseList* cases, SourceLocation* beginSelect, SourceLocation* endSelect);
    Select(Program* program, SourceLocation* location, Expression* expr, SourceLocation* beginSelect, SourceLocation* endSelect);

    Expression* expression() const;
    MaybeNull<CaseList> cases() const;
    SourceLocation* beginSelectLocation() const;
    SourceLocation* endSelectLocation() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Expression> expr_;
    Reference<CaseList> cases_;
    Reference<SourceLocation> beginLoc_;
    Reference<SourceLocation> endLoc_;
};

class ODBCOMPILER_PUBLIC_API CaseList final : public Node
{
public:
    CaseList(Program* program, SourceLocation* location, Case* case_);
    CaseList(Program* program, SourceLocation* location, DefaultCase* case_);
    CaseList(Program* program, SourceLocation* location);

    void appendCase(Case* case_);
    void appendDefaultCase(DefaultCase* case_);

    const std::vector<Reference<Case>>& cases() const;
    const std::vector<Reference<DefaultCase>>& defaultCases() const;
    MaybeNull<DefaultCase> defaultCase() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    std::vector<Reference<Case>> cases_;
    std::vector<Reference<DefaultCase>> defaults_;
};

class ODBCOMPILER_PUBLIC_API Case final : public Node
{
public:
    Case(Program* program, SourceLocation* location, Expression* expr, Block* body);
    Case(Program* program, SourceLocation* location, Expression* expr);

    Expression* expression() const;
    MaybeNull<Block> body() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Expression> expr_;
    Reference<Block> body_;
};

class ODBCOMPILER_PUBLIC_API DefaultCase final : public Node
{
public:
    DefaultCase(Program* program, SourceLocation* location, Block* body, SourceLocation* beginCaseLoc, SourceLocation* endCaseLoc);
    DefaultCase(Program* program, SourceLocation* location, SourceLocation* beginCaseLoc, SourceLocation* endCaseLoc);

    MaybeNull<Block> body() const;
    SourceLocation* beginCaseLocation() const;
    SourceLocation* endCaseLocation() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Block> body_;
    Reference<SourceLocation> beginLoc_;
    Reference<SourceLocation> endLoc_;
};

}
