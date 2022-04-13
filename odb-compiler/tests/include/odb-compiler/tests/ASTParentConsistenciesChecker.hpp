#pragma once

#include "odb-compiler/ast/Visitor.hpp"

class ASTParentConsistenciesChecker : public odb::ast::ConstVisitor
{
public:
    void visitAnnotatedSymbol(const odb::ast::AnnotatedSymbol* node) override;
    void visitArgList(const odb::ast::ArgList* node) override;
    void visitArrayAssignment(const odb::ast::ArrayAssignment* node) override;
    void visitArrayRef(const odb::ast::ArrayRef* node) override;
    void visitBinaryOp(const odb::ast::BinaryOp* node) override;
    void visitBlock(const odb::ast::Block* node) override;
    void visitCase(const odb::ast::Case* node) override;
    void visitCaseList(const odb::ast::CaseList* node) override;
    void visitCommandExpr(const odb::ast::CommandExpr* node) override;
    void visitCommandStmnt(const odb::ast::CommandStmnt* node) override;
    void visitConditional(const odb::ast::Conditional* node) override;
    void visitConstDecl(const odb::ast::ConstDecl* node) override;
    void visitConstDeclExpr(const odb::ast::ConstDeclExpr* node) override;
    void visitDefaultCase(const odb::ast::DefaultCase* node) override;
    void visitExit(const odb::ast::Exit* node) override;
    void visitForLoop(const odb::ast::ForLoop* node) override;
    void visitFuncCallExpr(const odb::ast::FuncCallExpr* node) override;
    void visitFuncCallExprOrArrayRef(const odb::ast::FuncCallExprOrArrayRef* node) override;
    void visitFuncCallStmnt(const odb::ast::FuncCallStmnt* node) override;
    void visitFuncDecl(const odb::ast::FuncDecl* node) override;
    void visitFuncExit(const odb::ast::FuncExit* node) override;
    void visitGoto(const odb::ast::Goto* node) override;
    void visitInfiniteLoop(const odb::ast::InfiniteLoop* node) override;
    void visitInitializerList(const odb::ast::InitializerList* node) override;
    void visitLabel(const odb::ast::Label* node) override;
    void visitScopedAnnotatedSymbol(const odb::ast::ScopedAnnotatedSymbol* node) override;
    void visitSelect(const odb::ast::Select* node) override;
    void visitSubCall(const odb::ast::SubCall* node) override;
    void visitSubReturn(const odb::ast::SubReturn* node) override;
    void visitSymbol(const odb::ast::Symbol* node) override;
    void visitVarDecl(const odb::ast::VarDecl* node) override;
    void visitUDTArrayDecl(const odb::ast::UDTArrayDecl* node) override;
    void visitUDTDecl(const odb::ast::UDTDecl* node) override;
    void visitUDTDeclBody(const odb::ast::UDTDeclBody* node) override;
    void visitUDTFieldAssignment(const odb::ast::UDTFieldAssignment* node) override;
    void visitUDTFieldOuter(const odb::ast::UDTFieldOuter* node) override;
    void visitUDTFieldInner(const odb::ast::UDTFieldInner* node) override;
    void visitUDTRef(const odb::ast::UDTRef* node) override;
    void visitUnaryOp(const odb::ast::UnaryOp* node) override;
    void visitUntilLoop(const odb::ast::UntilLoop* node) override;
    void visitVarAssignment(const odb::ast::VarAssignment* node) override;
    void visitVarRef(const odb::ast::VarRef* node) override;
    void visitWhileLoop(const odb::ast::WhileLoop* node) override;

#define X(dbname, cppname) \
    void visit##dbname##Literal(const odb::ast::dbname##Literal* node) override; \
    void visit##dbname##ArrayDecl(const odb::ast::dbname##ArrayDecl* node) override;
    ODB_DATATYPE_LIST
#undef X

};
