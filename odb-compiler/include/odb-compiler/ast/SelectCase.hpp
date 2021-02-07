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

class Select : public Statement
{
public:
    Select(Expression* expr, CaseList* cases, SourceLocation* location);
    Select(Expression* expr, SourceLocation* location);

    Expression* expression() const;
    MaybeNull<CaseList> cases() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Expression> expr_;
    Reference<CaseList> cases_;
};

class CaseList : public Node
{
public:
    CaseList(Case* case_, SourceLocation* location);
    CaseList(SourceLocation* location);

    void appendCase(Case* case_);
    void setDefaultCase(DefaultCase* case_);

    const std::vector<Reference<Case>>& cases() const;
    MaybeNull<DefaultCase> defaultCase() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    std::vector<Reference<Case>> cases_;
    Reference<DefaultCase> default_;
};

class Case : public Node
{
public:
    Case(Expression* expr, Block* body, SourceLocation* location);
    Case(Expression* expr, SourceLocation* location);

    Expression* expression() const;
    MaybeNull<Block> body() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Expression> expr_;
    Reference<Block> body_;
};

class DefaultCase : public Node
{
public:
    DefaultCase(Block* body, SourceLocation* location);
    DefaultCase(SourceLocation* location);

    MaybeNull<Block> body() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Block> body_;
};

}
