#pragma once

#include "odb-compiler/ast/Node.hpp"

namespace odb {
namespace ast {

class Visitor
{
public:
    virtual void visitBlock(const Block* node) = 0;
#define X(dbname, cppname) virtual void visit##dbname##Literal(const dbname##Literal* node) = 0;
    ODB_DATATYPE_LIST
#undef X
    virtual void visitSymbol(const Symbol* node) = 0;
    virtual void visitAnnotatedSymbol(const AnnotatedSymbol* node) = 0;
    virtual void visitScopedSymbol(const ScopedSymbol* node) = 0;
    virtual void visitScopedAnnotatedSymbol(const ScopedAnnotatedSymbol* node) = 0;
    virtual void visitConstDecl(const ConstDecl* node) = 0;
};

class GenericVisitor : public Visitor
{
public:
    void visitBlock(const Block* node) override;
#define X(dbname, cppname) void visit##dbname##Literal(const dbname##Literal* node) override;
    ODB_DATATYPE_LIST
#undef X
    void visitSymbol(const Symbol* node) override;
    void visitAnnotatedSymbol(const AnnotatedSymbol* node) override;
    void visitScopedSymbol(const ScopedSymbol* node) override;
    void visitScopedAnnotatedSymbol(const ScopedAnnotatedSymbol* node) override;
    void visitConstDecl(const ConstDecl* node) override;

    virtual void visit(const Node* node) = 0;
};

}
}
