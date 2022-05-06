#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-compiler/ast/VariableScope.hpp"
#include "odb-sdk/MaybeNull.hpp"

namespace odb::ast {

class Identifier;
class Block;
class Expression;
class ArgList;

class ODBCOMPILER_PUBLIC_API FuncDecl final : public Statement
{
public:
    FuncDecl(Identifier* identifier, ArgList* args, Block* body, Expression* returnValue, SourceLocation* location);
    FuncDecl(Identifier* identifier, ArgList* args, Expression* returnValue, SourceLocation* location);
    FuncDecl(Identifier* identifier, Block* body, Expression* returnValue, SourceLocation* location);
    FuncDecl(Identifier* identifier, Expression* returnValue, SourceLocation* location);
    FuncDecl(Identifier* identifier, ArgList* args, Block* body, SourceLocation* location);
    FuncDecl(Identifier* identifier, ArgList* args, SourceLocation* location);
    FuncDecl(Identifier* identifier, Block* body, SourceLocation* location);
    FuncDecl(Identifier* identifier, SourceLocation* location);

    Identifier* identifier() const;
    MaybeNull<ArgList> args() const;
    MaybeNull<Block> body() const;
    MaybeNull<Expression> returnValue() const;

    VariableScope& scope();
    const VariableScope& scope() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Identifier> identifier_;
    Reference<ArgList> args_;
    Reference<Block> body_;
    Reference<Expression> returnValue_;

    VariableScope scope_;
};

class ODBCOMPILER_PUBLIC_API FuncExit final : public Statement
{
public:
    FuncExit(Expression* returnValue, SourceLocation* location);
    FuncExit(SourceLocation* location);

    MaybeNull<Expression> returnValue() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    ChildRange children() override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<Expression> returnValue_;
};

}
