#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"

namespace odb::ast {

class Label;
class Identifier;

class ODBCOMPILER_PUBLIC_API UnresolvedSubCall final : public Statement
{
public:
    UnresolvedSubCall(Identifier* label, SourceLocation* location);

    Identifier* label() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Identifier> label_;
};

class ODBCOMPILER_PUBLIC_API SubCall final : public Statement
{
public:
    SubCall(Label* label, SourceLocation* location);

    Label* label() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Label> label_;
};

class ODBCOMPILER_PUBLIC_API SubReturn final : public Statement
{
public:
    SubReturn(SourceLocation* location);

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;
};

}
