#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"

namespace odb::ast {

class Label;
class Identifier;

class ODBCOMPILER_PUBLIC_API UnresolvedGoto final : public Statement
{
public:
    UnresolvedGoto(Program* program, SourceLocation* location, Identifier* label);

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

class ODBCOMPILER_PUBLIC_API Goto final : public Statement
{
public:
    Goto(Program* program, SourceLocation* location, Label* label);

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

}
