#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-sdk/MaybeNull.hpp"

namespace odb {
namespace ast {

class AnnotatedSymbol;
class Block;
class Expression;
class VarAssignment;

class Loop : public Statement
{
public:
    Loop(SourceLocation* location);
};

class InfiniteLoop : public Loop
{
public:
    InfiniteLoop(Block* body, SourceLocation* location);
    InfiniteLoop(SourceLocation* location);

    MaybeNull<Block> body() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<Block> body_;
};

class WhileLoop : public Loop
{
public:
    WhileLoop(Expression* continueCondition, Block* body, SourceLocation* location);
    WhileLoop(Expression* continueCondition, SourceLocation* location);

    Expression* continueCondition() const;
    MaybeNull<Block> body() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<Expression> continueCondition_;
    Reference<Block> body_;
};

class UntilLoop : public Loop
{
public:
    UntilLoop(Expression* exitCondition, Block* body, SourceLocation* location);
    UntilLoop(Expression* exitCondition, SourceLocation* location);

    Expression* exitCondition() const;
    MaybeNull<Block> body() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<Expression> exitCondition_;
    Reference<Block> body_;
};

class ForLoop : public Loop
{
public:
    ForLoop(VarAssignment* counter, Expression* endValue, Expression* stepValue, AnnotatedSymbol* nextSymbol, Block* body, SourceLocation* location);
    ForLoop(VarAssignment* counter, Expression* endValue, Expression* stepValue, AnnotatedSymbol* nextSymbol, SourceLocation* location);
    ForLoop(VarAssignment* counter, Expression* endValue, Expression* stepValue, Block* body, SourceLocation* location);
    ForLoop(VarAssignment* counter, Expression* endValue, Expression* stepValue, SourceLocation* location);
    ForLoop(VarAssignment* counter, Expression* endValue, AnnotatedSymbol* nextSymbol, Block* body, SourceLocation* location);
    ForLoop(VarAssignment* counter, Expression* endValue, AnnotatedSymbol* nextSymbol, SourceLocation* location);
    ForLoop(VarAssignment* counter, Expression* endValue, Block* body, SourceLocation* location);
    ForLoop(VarAssignment* counter, Expression* endValue, SourceLocation* location);

    VarAssignment* counter() const;
    Expression* endValue() const;
    MaybeNull<Expression> stepValue() const;
    MaybeNull<AnnotatedSymbol> nextSymbol() const;
    MaybeNull<Block> body() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<VarAssignment> counter_;
    Reference<Expression> endValue_;
    Reference<Expression> stepValue_;
    Reference<AnnotatedSymbol> nextSymbol_;
    Reference<Block> body_;
};

}
}
