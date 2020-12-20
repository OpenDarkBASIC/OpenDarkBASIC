#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-sdk/MaybeNull.hpp"

namespace odb {
namespace ast {

class AnnotatedSymbol;
class ExpressionList;

class FuncCallExpr : public Expression
{
public:
    FuncCallExpr(AnnotatedSymbol* symbol, ExpressionList* args, SourceLocation* location);
    FuncCallExpr(AnnotatedSymbol* symbol, SourceLocation* location);

    AnnotatedSymbol* symbol() const;
    MaybeNull<ExpressionList> args() const;

    void accept(Visitor* visitor) const override;

private:
    Reference<AnnotatedSymbol> symbol_;
    Reference<ExpressionList> args_;
};

class FuncCallStmnt : public Statement
{
public:
    FuncCallStmnt(AnnotatedSymbol* symbol, ExpressionList* args, SourceLocation* location);
    FuncCallStmnt(AnnotatedSymbol* symbol, SourceLocation* location);

    AnnotatedSymbol* symbol() const;
    MaybeNull<ExpressionList> args() const;

    void accept(Visitor* visitor) const override;

private:
    Reference<AnnotatedSymbol> symbol_;
    Reference<ExpressionList> args_;
};

/*!
 * It's not possible to determine whether
 *
 *   foo(3, 4)
 *
 * is a function call or an array access. This class represents such an entity.
 * This is fixed in a second stage later.
 */
class FuncCallExprOrArrayRef : public Expression
{
public:
    FuncCallExprOrArrayRef(AnnotatedSymbol* symbol, ExpressionList* args, SourceLocation* location);

    AnnotatedSymbol* symbol() const;
    MaybeNull<ExpressionList> args() const;

    void accept(Visitor* visitor) const override;

private:
    Reference<AnnotatedSymbol> symbol_;
    Reference<ExpressionList> args_;
};

}
}
