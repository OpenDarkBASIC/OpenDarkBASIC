#pragma once

#include "odb-compiler/ast/Node.hpp"

namespace odb {
namespace ast {

class Visitor
{
public:
    virtual void visitBlock(const Block* node) = 0;
    virtual void visitExpressionList(const ExpressionList* node) = 0;
#define X(dbname, cppname) virtual void visit##dbname##Literal(const dbname##Literal* node) = 0;
    ODB_DATATYPE_LIST
#undef X
    virtual void visitSymbol(const Symbol* node) = 0;
    virtual void visitAnnotatedSymbol(const AnnotatedSymbol* node) = 0;
    virtual void visitScopedSymbol(const ScopedSymbol* node) = 0;
    virtual void visitScopedAnnotatedSymbol(const ScopedAnnotatedSymbol* node) = 0;
    virtual void visitFuncCallExprOrArrayRef(const FuncCallExprOrArrayRef* node)  = 0;
    virtual void visitFuncCallExpr(const FuncCallExpr* node) = 0;
    virtual void visitFuncCallStmnt(const FuncCallStmnt* node) = 0;
    virtual void visitArrayRef(const ArrayRef* node)  = 0;
    virtual void visitConstDecl(const ConstDecl* node) = 0;
    virtual void visitKeywordExprSymbol(const KeywordExprSymbol* node) = 0;
    virtual void visitKeywordStmntSymbol(const KeywordStmntSymbol* node) = 0;
    virtual void visitKeywordExpr(const KeywordExpr* node) = 0;
    virtual void visitKeywordStmnt(const KeywordStmnt* node) = 0;
};

class GenericVisitor : public Visitor
{
public:
    void visitBlock(const Block* node) override;
    void visitExpressionList(const ExpressionList* node) override;
#define X(dbname, cppname) void visit##dbname##Literal(const dbname##Literal* node) override;
    ODB_DATATYPE_LIST
#undef X
    void visitSymbol(const Symbol* node) override;
    void visitAnnotatedSymbol(const AnnotatedSymbol* node) override;
    void visitScopedSymbol(const ScopedSymbol* node) override;
    void visitScopedAnnotatedSymbol(const ScopedAnnotatedSymbol* node) override;
    void visitFuncCallExprOrArrayRef(const FuncCallExprOrArrayRef* node) override;
    void visitFuncCallExpr(const FuncCallExpr* node) override;
    void visitFuncCallStmnt(const FuncCallStmnt* node) override;
    void visitArrayRef(const ArrayRef* node) override;
    void visitConstDecl(const ConstDecl* node) override;
    void visitKeywordExprSymbol(const KeywordExprSymbol* node) override;
    void visitKeywordStmntSymbol(const KeywordStmntSymbol* node) override;
    void visitKeywordExpr(const KeywordExpr* node) override;
    void visitKeywordStmnt(const KeywordStmnt* node) override;

    virtual void visit(const Node* node) = 0;
};

}
}
