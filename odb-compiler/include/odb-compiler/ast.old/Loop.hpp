#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-util/MaybeNull.hpp"

namespace odb::ast {

class Identifier;
class Block;
class Expression;
class Assignment;

class ODBCOMPILER_PUBLIC_API Loop : public Statement
{
public:
    Loop(Program* program, SourceLocation* location);
};

class ODBCOMPILER_PUBLIC_API InfiniteLoop final : public Loop
{
public:
    InfiniteLoop(Program* program, SourceLocation* location, Block* body);
    InfiniteLoop(Program* program, SourceLocation* location);

    MaybeNull<Block> body() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Block> body_;
};

class ODBCOMPILER_PUBLIC_API WhileLoop final : public Loop
{
public:
    WhileLoop(Program* program, SourceLocation* location, Expression* continueCondition, Block* body);
    WhileLoop(Program* program, SourceLocation* location, Expression* continueCondition);

    Expression* continueCondition() const;
    MaybeNull<Block> body() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Expression> continueCondition_;
    Reference<Block> body_;
};

class ODBCOMPILER_PUBLIC_API UntilLoop final : public Loop
{
public:
    UntilLoop(Program* program, SourceLocation* location, Expression* exitCondition, Block* body);
    UntilLoop(Program* program, SourceLocation* location, Expression* exitCondition);

    Expression* exitCondition() const;
    MaybeNull<Block> body() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Expression> exitCondition_;
    Reference<Block> body_;
};

class ODBCOMPILER_PUBLIC_API ForLoop final : public Loop
{
public:
    ForLoop(Program* program, SourceLocation* location, Assignment* counter, Expression* endValue, Expression* stepValue, Identifier* nextIdentifier, Block* body);
    ForLoop(Program* program, SourceLocation* location, Assignment* counter, Expression* endValue, Expression* stepValue, Identifier* nextIdentifier);
    ForLoop(Program* program, SourceLocation* location, Assignment* counter, Expression* endValue, Expression* stepValue, Block* body);
    ForLoop(Program* program, SourceLocation* location, Assignment* counter, Expression* endValue, Expression* stepValue);
    ForLoop(Program* program, SourceLocation* location, Assignment* counter, Expression* endValue, Identifier* nextIdentifier, Block* body);
    ForLoop(Program* program, SourceLocation* location, Assignment* counter, Expression* endValue, Identifier* nextIdentifier);
    ForLoop(Program* program, SourceLocation* location, Assignment* counter, Expression* endValue, Block* body);
    ForLoop(Program* program, SourceLocation* location, Assignment* counter, Expression* endValue);

    Assignment* counter() const;
    Expression* endValue() const;
    Expression* stepValue() const;
    MaybeNull<Identifier> nextIdentifier() const;
    MaybeNull<Block> body() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Assignment> counter_;
    Reference<Expression> endValue_;
    Reference<Expression> stepValue_;
    Reference<Identifier> nextIdentifier_;
    Reference<Block> body_;
};

}
