#pragma once

#include "odb-compiler/ast/Visitor.hpp"

class ASTParentConsistenciesChecker : public odb::ast::ConstVisitor
{
public:
    void visitAnnotatedSymbol(const odb::ast::AnnotatedSymbol* node) override;
    void visitArrayAssignment(const odb::ast::ArrayAssignment* node) override;
    void visitArrayRef(const odb::ast::ArrayRef* node) override;
    void visitBlock(const odb::ast::Block* node) override;
    void visitBreak(const odb::ast::Break* node) override;
    void visitCommandExpr(const odb::ast::CommandExpr* node) override;
    void visitCommandExprSymbol(const odb::ast::CommandExprSymbol* node) override;
    void visitCommandStmnt(const odb::ast::CommandStmnt* node) override;
    void visitCommandStmntSymbol(const odb::ast::CommandStmntSymbol* node) override;
    void visitConditional(const odb::ast::Conditional* node) override;
    void visitConstDecl(const odb::ast::ConstDecl* node) override;
    void visitDecrementVar(const odb::ast::DecrementVar* node) override;
    void visitExpressionList(const odb::ast::ExpressionList* node) override;
    void visitForLoop(const odb::ast::ForLoop* node) override;
    void visitFuncCallExpr(const odb::ast::FuncCallExpr* node) override;
    void visitFuncCallExprOrArrayRef(const odb::ast::FuncCallExprOrArrayRef* node) override;
    void visitFuncCallStmnt(const odb::ast::FuncCallStmnt* node) override;
    void visitFuncDecl(const odb::ast::FuncDecl* node) override;
    void visitFuncExit(const odb::ast::FuncExit* node) override;
    void visitGoto(const odb::ast::Goto* node) override;
    void visitGotoSymbol(const odb::ast::GotoSymbol* node) override;
    void visitIncrementVar(const odb::ast::IncrementVar* node) override;
    void visitInfiniteLoop(const odb::ast::InfiniteLoop* node) override;
    void visitLabel(const odb::ast::Label* node) override;
    void visitScopedSymbol(const odb::ast::ScopedSymbol* node) override;
    void visitScopedAnnotatedSymbol(const odb::ast::ScopedAnnotatedSymbol* node) override;
    void visitSubCall(const odb::ast::SubCall* node) override;
    void visitSubCallSymbol(const odb::ast::SubCallSymbol* node) override;
    void visitSubReturn(const odb::ast::SubReturn* node) override;
    void visitSymbol(const odb::ast::Symbol* node) override;
    void visitUDTArrayDecl(const odb::ast::UDTArrayDecl* node) override;
    void visitUDTArrayDeclSymbol(const odb::ast::UDTArrayDeclSymbol* node) override;
    void visitUDTArrayRef(const odb::ast::UDTArrayRef* node) override;
    void visitUDTDecl(const odb::ast::UDTDecl* node) override;
    void visitUDTDeclBody(const odb::ast::UDTDeclBody* node) override;
    void visitUDTRef(const odb::ast::UDTRef* node) override;
    void visitUDTVarDecl(const odb::ast::UDTVarDecl* node) override;
    void visitUDTVarDeclSymbol(const odb::ast::UDTVarDeclSymbol* node) override;
    void visitUDTVarRef(const odb::ast::UDTVarRef* node) override;
    void visitUntilLoop(const odb::ast::UntilLoop* node) override;
    void visitVarAssignment(const odb::ast::VarAssignment* node) override;
    void visitVarRef(const odb::ast::VarRef* node) override;
    void visitWhileLoop(const odb::ast::WhileLoop* node) override;

#define X(dbname, cppname) \
    void visit##dbname##Literal(const odb::ast::dbname##Literal* node) override; \
    void visit##dbname##VarDecl(const odb::ast::dbname##VarDecl* node) override; \
    void visit##dbname##ArrayDecl(const odb::ast::dbname##ArrayDecl* node) override;
    ODB_DATATYPE_LIST
#undef X

#define X(op, tok) \
    void visitBinaryOp##op(const odb::ast::BinaryOp##op* node) override;
    ODB_BINARY_OP_LIST
#undef X

#define X(op, tok) \
    void visitUnaryOp##op(const odb::ast::UnaryOp##op* node) override;
    ODB_UNARY_OP_LIST
#undef X
};
