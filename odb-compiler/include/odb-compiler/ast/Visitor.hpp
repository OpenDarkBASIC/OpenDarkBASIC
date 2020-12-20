#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Datatypes.hpp"
#include "odb-compiler/ast/BinaryOperators.hpp"

namespace odb {
namespace ast {

class Node;

class AnnotatedSymbol;
class ArrayRef;
class Block;
class Break;
class ConstDecl;
class ExpressionList;
class ForLoop;
class FuncCallExpr;
class FuncCallExprOrArrayRef;
class FuncCallStmnt;
class FuncDecl;
class FuncExit;
class InfiniteLoop;
class KeywordExpr;
class KeywordExprSymbol;
class KeywordStmnt;
class KeywordStmntSymbol;
class ScopedSymbol;
class ScopedAnnotatedSymbol;
class Symbol;
class UntilLoop;
class VarAssignment;
class VarRef;
class WhileLoop;

template <typename T> class LiteralTemplate;
template <typename T> class VarDeclTemplate;

#define X(dbname, cppname) \
    typedef LiteralTemplate<cppname> dbname##Literal; \
    typedef VarDeclTemplate<cppname> dbname##VarDecl;
ODB_DATATYPE_LIST
#undef X

#define X(op, str) class BinaryOp##op;
ODB_BINARY_OP_LIST
#undef X

class ODBCOMPILER_PUBLIC_API Visitor
{
public:
    virtual void visitAnnotatedSymbol(const AnnotatedSymbol* node) = 0;
    virtual void visitArrayRef(const ArrayRef* node)  = 0;
    virtual void visitBlock(const Block* node) = 0;
    virtual void visitBreak(const Break* node) = 0;
    virtual void visitConstDecl(const ConstDecl* node) = 0;
    virtual void visitExpressionList(const ExpressionList* node) = 0;
    virtual void visitForLoop(const ForLoop* node) = 0;
    virtual void visitFuncCallExpr(const FuncCallExpr* node) = 0;
    virtual void visitFuncCallExprOrArrayRef(const FuncCallExprOrArrayRef* node)  = 0;
    virtual void visitFuncCallStmnt(const FuncCallStmnt* node) = 0;
    virtual void visitFuncDecl(const FuncDecl* node) = 0;
    virtual void visitFuncExit(const FuncExit* node) = 0;
    virtual void visitInfiniteLoop(const InfiniteLoop* node) = 0;
    virtual void visitKeywordExpr(const KeywordExpr* node) = 0;
    virtual void visitKeywordExprSymbol(const KeywordExprSymbol* node) = 0;
    virtual void visitKeywordStmnt(const KeywordStmnt* node) = 0;
    virtual void visitKeywordStmntSymbol(const KeywordStmntSymbol* node) = 0;
    virtual void visitScopedSymbol(const ScopedSymbol* node) = 0;
    virtual void visitScopedAnnotatedSymbol(const ScopedAnnotatedSymbol* node) = 0;
    virtual void visitSymbol(const Symbol* node) = 0;
    virtual void visitUntilLoop(const UntilLoop* node) = 0;
    virtual void visitVarAssignment(const VarAssignment* node) = 0;
    virtual void visitVarRef(const VarRef* node) = 0;
    virtual void visitWhileLoop(const WhileLoop* node) = 0;

#define X(dbname, cppname) \
    virtual void visit##dbname##Literal(const dbname##Literal* node) = 0; \
    virtual void visit##dbname##VarDecl(const dbname##VarDecl* node) = 0;
    ODB_DATATYPE_LIST
#undef X

#define X(op, str) \
    virtual void visitBinaryOp##op(const BinaryOp##op* node) = 0;
    ODB_BINARY_OP_LIST
#undef X
};

class ODBCOMPILER_PUBLIC_API GenericVisitor : public Visitor
{
public:
    void visitAnnotatedSymbol(const AnnotatedSymbol* node) override;
    void visitArrayRef(const ArrayRef* node) override;
    void visitBlock(const Block* node) override;
    void visitBreak(const Break* node) override;
    void visitConstDecl(const ConstDecl* node) override;
    void visitExpressionList(const ExpressionList* node) override;
    void visitForLoop(const ForLoop* node) override;
    void visitFuncCallExpr(const FuncCallExpr* node) override;
    void visitFuncCallExprOrArrayRef(const FuncCallExprOrArrayRef* node) override;
    void visitFuncCallStmnt(const FuncCallStmnt* node) override;
    void visitFuncDecl(const FuncDecl* node) override;
    void visitFuncExit(const FuncExit* node) override;
    void visitInfiniteLoop(const InfiniteLoop* node) override;
    void visitKeywordExpr(const KeywordExpr* node) override;
    void visitKeywordExprSymbol(const KeywordExprSymbol* node) override;
    void visitKeywordStmnt(const KeywordStmnt* node) override;
    void visitKeywordStmntSymbol(const KeywordStmntSymbol* node) override;
    void visitScopedSymbol(const ScopedSymbol* node) override;
    void visitScopedAnnotatedSymbol(const ScopedAnnotatedSymbol* node) override;
    void visitSymbol(const Symbol* node) override;
    void visitUntilLoop(const UntilLoop* node) override;
    void visitVarAssignment(const VarAssignment* node) override;
    void visitVarRef(const VarRef* node) override;
    void visitWhileLoop(const WhileLoop* node) override;

#define X(dbname, cppname) \
    void visit##dbname##Literal(const dbname##Literal* node) override; \
    void visit##dbname##VarDecl(const dbname##VarDecl* node) override;
    ODB_DATATYPE_LIST
#undef X

#define X(op, str) \
    void visitBinaryOp##op(const BinaryOp##op* node) override;
    ODB_BINARY_OP_LIST
#undef X

    virtual void visit(const Node* node) = 0;
};

}
}
