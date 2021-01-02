#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-compiler/ast/Datatypes.hpp"

namespace odb {
namespace ast {

class ExpressionList;
class ScopedAnnotatedSymbol;
class Symbol;
class UDTRef;

class ODBCOMPILER_PUBLIC_API ArrayDecl : public Statement
{
public:
    ArrayDecl(SourceLocation* location);
};

template <typename T>
class ArrayDeclTemplate : public ArrayDecl
{
public:
    ArrayDeclTemplate(ScopedAnnotatedSymbol* symbol, ExpressionList* dims, SourceLocation* location);

    ScopedAnnotatedSymbol* symbol() const;
    ExpressionList* dims() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<ScopedAnnotatedSymbol> symbol_;
    Reference<ExpressionList> dims_;
};

#define X(dbname, cppname) \
    template class ODBCOMPILER_PUBLIC_API ArrayDeclTemplate<cppname>; \
    typedef ArrayDeclTemplate<cppname> dbname##ArrayDecl;
ODB_DATATYPE_LIST
#undef X

class ODBCOMPILER_PUBLIC_API UDTArrayDeclSymbol : public ArrayDecl
{
public:
    UDTArrayDeclSymbol(ScopedAnnotatedSymbol* symbol, ExpressionList* dims, Symbol* udt, SourceLocation* location);

    ScopedAnnotatedSymbol* symbol() const;
    ExpressionList* dims() const;
    Symbol* udtSymbol() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<ScopedAnnotatedSymbol> symbol_;
    Reference<ExpressionList> dims_;
    Reference<Symbol> udt_;
};

class ODBCOMPILER_PUBLIC_API UDTArrayDecl : public ArrayDecl
{
public:
    UDTArrayDecl(ScopedAnnotatedSymbol* symbol, ExpressionList* dims, UDTRef* udt, SourceLocation* location);

    ScopedAnnotatedSymbol* symbol() const;
    ExpressionList* dims() const;
    UDTRef* udt() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

private:
    Reference<ScopedAnnotatedSymbol> symbol_;
    Reference<ExpressionList> dims_;
    Reference<UDTRef> udt_;
};

}
}
