#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"

namespace odb::ast {

class Label;
class Identifier;

class ODBCOMPILER_PUBLIC_API UnresolvedSubCall final : public Statement
{
public:
    UnresolvedSubCall(Program* program, SourceLocation* location, Identifier* label);

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
    SubCall(Program* program, SourceLocation* location, Label* label);

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
    SubReturn(Program* program, SourceLocation* location);

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;
};

}
