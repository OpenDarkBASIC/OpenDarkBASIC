#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-compiler/ast/Datatypes.hpp"

namespace odb {
namespace ast {

class Expression;
class UDTRef;
class ScopedAnnotatedSymbol;
class Symbol;

class ODBCOMPILER_PUBLIC_API VarDecl : public Statement
{
public:
    VarDecl(SourceLocation* location);
    virtual void setInitialValue(Expression* expression) = 0;
    virtual ScopedAnnotatedSymbol* symbol() const = 0;
};

template <typename T>
class VarDeclTemplate : public VarDecl
{
public:
    VarDeclTemplate(ScopedAnnotatedSymbol* symbol, Expression* initialValue, SourceLocation* location);
    VarDeclTemplate(ScopedAnnotatedSymbol* symbol, SourceLocation* location);

    void setInitialValue(Expression* expression) override;

    ScopedAnnotatedSymbol* symbol() const override;
    Expression* initialValue() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<ScopedAnnotatedSymbol> symbol_;
    Reference<Expression> initialValue_;
};

#define X(dbname, cppname) \
    template class ODBCOMPILER_PUBLIC_API VarDeclTemplate<cppname>; \
    typedef VarDeclTemplate<cppname> dbname##VarDecl;
ODB_DATATYPE_LIST
#undef X

class ODBCOMPILER_PUBLIC_API UDTVarDeclSymbol : public VarDecl
{
public:
    UDTVarDeclSymbol(ScopedAnnotatedSymbol* symbol, Symbol* udt, SourceLocation* location);

    void setInitialValue(Expression* expression) override;

    ScopedAnnotatedSymbol* symbol() const override;
    Symbol* udtSymbol() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<ScopedAnnotatedSymbol> symbol_;
    Reference<Symbol> udt_;
};

class ODBCOMPILER_PUBLIC_API UDTVarDecl : public VarDecl
{
public:
    UDTVarDecl(ScopedAnnotatedSymbol* symbol, UDTRef* udt, SourceLocation* location);
    ~UDTVarDecl();

    void setInitialValue(Expression* expression) override;

    ScopedAnnotatedSymbol* symbol() const override;
    UDTRef* udt() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<ScopedAnnotatedSymbol> symbol_;
    Reference<UDTRef> udt_;
};

}
}
