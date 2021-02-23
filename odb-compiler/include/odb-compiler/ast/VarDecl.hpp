#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-compiler/ast/Datatypes.hpp"
#include "odb-sdk/MaybeNull.hpp"

namespace odb {
namespace ast {

class ExpressionList;
class UDTRef;
class ScopedAnnotatedSymbol;
class Symbol;

class ODBCOMPILER_PUBLIC_API VarDecl : public Statement
{
public:
    VarDecl(ScopedAnnotatedSymbol* symbol, ExpressionList* initializer, SourceLocation* location);
    VarDecl(ScopedAnnotatedSymbol* symbol, SourceLocation* location);

    ScopedAnnotatedSymbol* symbol() const;
    MaybeNull<ExpressionList> initializer() const;

    void setInitializer(ExpressionList* expression);

protected:
    Reference<ScopedAnnotatedSymbol> symbol_;
    Reference<ExpressionList> initializer_;
};

template <typename T>
class VarDeclTemplate : public VarDecl
{
public:
    VarDeclTemplate(ScopedAnnotatedSymbol* symbol, ExpressionList* initializer, SourceLocation* location);
    VarDeclTemplate(ScopedAnnotatedSymbol* symbol, SourceLocation* location);

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;
};

#define X(dbname, cppname) \
    template class ODBCOMPILER_PUBLIC_API VarDeclTemplate<cppname>; \
    typedef VarDeclTemplate<cppname> dbname##VarDecl;
ODB_DATATYPE_LIST
#undef X

class ODBCOMPILER_PUBLIC_API UDTVarDecl : public VarDecl
{
public:
    UDTVarDecl(ScopedAnnotatedSymbol* symbol, UDTRef* udt, ExpressionList* initializer, SourceLocation* location);
    UDTVarDecl(ScopedAnnotatedSymbol* symbol, UDTRef* udt, SourceLocation* location);

    UDTRef* udt() const;

    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<UDTRef> udt_;
};

}
}
