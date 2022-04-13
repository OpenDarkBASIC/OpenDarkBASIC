#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-sdk/MaybeNull.hpp"

namespace odb::ast {

class AnnotatedSymbol;
class ArgList;

class ODBCOMPILER_PUBLIC_API FuncCallExpr : public Expression
{
public:
    FuncCallExpr(AnnotatedSymbol* symbol, ArgList* args, SourceLocation* location);
    FuncCallExpr(AnnotatedSymbol* symbol, SourceLocation* location);

    AnnotatedSymbol* symbol() const;
    MaybeNull<ArgList> args() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<AnnotatedSymbol> symbol_;
    Reference<ArgList> args_;
};

class ODBCOMPILER_PUBLIC_API FuncCallStmnt : public Statement
{
public:
    FuncCallStmnt(AnnotatedSymbol* symbol, ArgList* args, SourceLocation* location);
    FuncCallStmnt(AnnotatedSymbol* symbol, SourceLocation* location);

    AnnotatedSymbol* symbol() const;
    MaybeNull<ArgList> args() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<AnnotatedSymbol> symbol_;
    Reference<ArgList> args_;
};

/*!
 * It's not possible to determine whether
 *
 *   foo(3, 4)
 *
 * is a function call or an array access. This class represents such an entity.
 * This is fixed in a second stage later.
 */
class ODBCOMPILER_PUBLIC_API FuncCallExprOrArrayRef : public Expression
{
public:
    FuncCallExprOrArrayRef(AnnotatedSymbol* symbol, ArgList* args, SourceLocation* location);

    AnnotatedSymbol* symbol() const;
    MaybeNull<ArgList> args() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<AnnotatedSymbol> symbol_;
    Reference<ArgList> args_;
};

}
