#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-sdk/MaybeNull.hpp"

namespace odb::ast {

class Identifier;
class ArgList;
class FuncDecl;

class ODBCOMPILER_PUBLIC_API FuncCallExpr final : public Expression
{
public:
    FuncCallExpr(Program* program, SourceLocation* location, Identifier* identifier, ArgList* args);
    FuncCallExpr(Program* program, SourceLocation* location, Identifier* identifier);

    Identifier* identifier() const;
    MaybeNull<ArgList> args() const;

    FuncDecl* function() const;
    void setFunction(FuncDecl* func);

    Type getType() const override;

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

    // Resolved in a later pass.
    FuncDecl* function_;
};

class ODBCOMPILER_PUBLIC_API FuncCallStmnt final : public Statement
{
public:
    FuncCallStmnt(Program* program, SourceLocation* location, Identifier* identifier, ArgList* args);
    FuncCallStmnt(Program* program, SourceLocation* location, Identifier* identifier);

    Identifier* identifier() const;
    MaybeNull<ArgList> args() const;

    FuncDecl* function() const;
    void setFunction(FuncDecl* func);

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

    // Resolved in a later pass.
    FuncDecl* function_;
};

/*!
 * It's not possible to determine whether
 *
 *   foo(3, 4)
 *
 * is a function call or an array access. This class represents such an entity.
 * This is replaced with either a FuncCallExpr or ArrayRef in a later pass.
 */
class ODBCOMPILER_PUBLIC_API FuncCallExprOrArrayRef final : public Expression
{
public:
    FuncCallExprOrArrayRef(Program* program, SourceLocation* location, Identifier* identifier, ArgList* args);

    Identifier* identifier() const;
    MaybeNull<ArgList> args() const;

    Type getType() const override;

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
};

}
