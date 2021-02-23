#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Statement.hpp"
#include "odb-compiler/ast/Datatypes.hpp"

namespace odb::ast {

class ArgList;
class ScopedAnnotatedSymbol;
class Symbol;
class UDTRef;

class ODBCOMPILER_PUBLIC_API ArrayDecl : public Statement
{
public:
    ArrayDecl(ScopedAnnotatedSymbol* symbol, ArgList* dims, SourceLocation* location);

    ScopedAnnotatedSymbol* symbol() const;
    ArgList* dims() const;

protected:
    Reference<ScopedAnnotatedSymbol> symbol_;
    Reference<ArgList> dims_;
};

template <typename T>
class ArrayDeclTemplate : public ArrayDecl
{
public:
    ArrayDeclTemplate(ScopedAnnotatedSymbol* symbol, ArgList* dims, SourceLocation* location);

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;
};

#define X(dbname, cppname) \
    template class ODBCOMPILER_PUBLIC_API ArrayDeclTemplate<cppname>; \
    typedef ArrayDeclTemplate<cppname> dbname##ArrayDecl;
ODB_DATATYPE_LIST
#undef X

class ODBCOMPILER_PUBLIC_API UDTArrayDecl : public ArrayDecl
{
public:
    UDTArrayDecl(ScopedAnnotatedSymbol* symbol, ArgList* dims, UDTRef* udt, SourceLocation* location);

    UDTRef* udt() const;

    std::string toString() const override;
    void accept(Visitor* visitor) override;
    void accept(ConstVisitor* visitor) const override;
    void swapChild(const Node* oldNode, Node* newNode) override;

protected:
    Node* duplicateImpl() const override;

private:
    Reference<UDTRef> udt_;
};

}
