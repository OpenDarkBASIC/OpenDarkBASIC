#pragma once

#include "odb-compiler/ast/Visitor.hpp"

class ASTParentConsistenciesChecker : public odb::ast::Visitor
{
public:
    void visitAnnotatedSymbol(const odb::ast::AnnotatedSymbol* node) override;
    void visitArrayRef(const odb::ast::ArrayRef* node) override;
    void visitBlock(const odb::ast::Block* node) override;
    void visitBreak(const odb::ast::Break* node) override;
    void visitConstDecl(const odb::ast::ConstDecl* node) override;
    void visitExpressionList(const odb::ast::ExpressionList* node) override;
    void visitForLoop(const odb::ast::ForLoop* node) override;
    void visitFuncCallExpr(const odb::ast::FuncCallExpr* node) override;
    void visitFuncCallExprOrArrayRef(const odb::ast::FuncCallExprOrArrayRef* node) override;
    void visitFuncCallStmnt(const odb::ast::FuncCallStmnt* node) override;
    void visitInfiniteLoop(const odb::ast::InfiniteLoop* node) override;
    void visitKeywordExpr(const odb::ast::KeywordExpr* node) override;
    void visitKeywordExprSymbol(const odb::ast::KeywordExprSymbol* node) override;
    void visitKeywordStmnt(const odb::ast::KeywordStmnt* node) override;
    void visitKeywordStmntSymbol(const odb::ast::KeywordStmntSymbol* node) override;
    void visitScopedSymbol(const odb::ast::ScopedSymbol* node) override;
    void visitScopedAnnotatedSymbol(const odb::ast::ScopedAnnotatedSymbol* node) override;
    void visitSymbol(const odb::ast::Symbol* node) override;
    void visitUntilLoop(const odb::ast::UntilLoop* node) override;
    void visitVarAssignment(const odb::ast::VarAssignment* node) override;
    void visitVarRef(const odb::ast::VarRef* node) override;
    void visitWhileLoop(const odb::ast::WhileLoop* node) override;

#define X(dbname, cppname) \
    void visit##dbname##Literal(const odb::ast::dbname##Literal* node) override; \
    void visit##dbname##VarDecl(const odb::ast::dbname##VarDecl* node) override;
    ODB_DATATYPE_LIST
#undef X
};
