#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-sdk/MaybeNull.hpp"

namespace odb::ast {

class AnnotatedSymbol;
class Block;
class Expression;
class Assignment;

class ODBCOMPILER_PUBLIC_API Loop : public Statement
{
public:
    Loop(SourceLocation* location);
};

class ODBCOMPILER_PUBLIC_API InfiniteLoop final : public Loop
{
public:
    InfiniteLoop(Block* body, SourceLocation* location);
    InfiniteLoop(SourceLocation* location);

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
    WhileLoop(Expression* continueCondition, Block* body, SourceLocation* location);
    WhileLoop(Expression* continueCondition, SourceLocation* location);

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
    UntilLoop(Expression* exitCondition, Block* body, SourceLocation* location);
    UntilLoop(Expression* exitCondition, SourceLocation* location);

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
    ForLoop(Assignment* counter, Expression* endValue, Expression* stepValue, AnnotatedSymbol* nextSymbol, Block* body, SourceLocation* location);
    ForLoop(Assignment* counter, Expression* endValue, Expression* stepValue, AnnotatedSymbol* nextSymbol, SourceLocation* location);
    ForLoop(Assignment* counter, Expression* endValue, Expression* stepValue, Block* body, SourceLocation* location);
    ForLoop(Assignment* counter, Expression* endValue, Expression* stepValue, SourceLocation* location);
    ForLoop(Assignment* counter, Expression* endValue, AnnotatedSymbol* nextSymbol, Block* body, SourceLocation* location);
    ForLoop(Assignment* counter, Expression* endValue, AnnotatedSymbol* nextSymbol, SourceLocation* location);
    ForLoop(Assignment* counter, Expression* endValue, Block* body, SourceLocation* location);
    ForLoop(Assignment* counter, Expression* endValue, SourceLocation* location);

    Assignment* counter() const;
    Expression* endValue() const;
    MaybeNull<Expression> stepValue() const;
    MaybeNull<AnnotatedSymbol> nextSymbol() const;
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
    Reference<AnnotatedSymbol> nextSymbol_;
    Reference<Block> body_;
};

}
