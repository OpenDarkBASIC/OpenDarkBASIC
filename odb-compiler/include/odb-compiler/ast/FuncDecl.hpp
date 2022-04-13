#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-sdk/MaybeNull.hpp"

namespace odb::ast {

class AnnotatedSymbol;
class Block;
class Expression;
class ArgList;

class ODBCOMPILER_PUBLIC_API FuncDecl : public Statement
{
public:
    FuncDecl(AnnotatedSymbol* symbol, ArgList* args, Block* body, Expression* returnValue, SourceLocation* location);
    FuncDecl(AnnotatedSymbol* symbol, ArgList* args, Expression* returnValue, SourceLocation* location);
    FuncDecl(AnnotatedSymbol* symbol, Block* body, Expression* returnValue, SourceLocation* location);
    FuncDecl(AnnotatedSymbol* symbol, Expression* returnValue, SourceLocation* location);
    FuncDecl(AnnotatedSymbol* symbol, ArgList* args, Block* body, SourceLocation* location);
    FuncDecl(AnnotatedSymbol* symbol, ArgList* args, SourceLocation* location);
    FuncDecl(AnnotatedSymbol* symbol, Block* body, SourceLocation* location);
    FuncDecl(AnnotatedSymbol* symbol, SourceLocation* location);

    AnnotatedSymbol* symbol() const;
    MaybeNull<ArgList> args() const;
    MaybeNull<Block> body() const;
    MaybeNull<Expression> returnValue() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<AnnotatedSymbol> symbol_;
    Reference<ArgList> args_;
    Reference<Block> body_;
    Reference<Expression> returnValue_;
};

class ODBCOMPILER_PUBLIC_API FuncExit : public Statement
{
public:
    FuncExit(Expression* returnValue, SourceLocation* location);
    FuncExit(SourceLocation* location);

    MaybeNull<Expression> returnValue() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Expression> returnValue_;
};

}
