#pragma once

#include "gmock/gmock.h"
#include "odb-compiler/ast/Visitor.hpp"

class ASTMockVisitor : public odb::ast::ConstVisitor
{
public:
    MOCK_METHOD(void, visitArgList, (const odb::ast::ArgList* node), (override));
    MOCK_METHOD(void, visitArrayAssignment, (const odb::ast::ArrayAssignment* node), (override));
    MOCK_METHOD(void, visitArrayDecl, (const odb::ast::ArrayDecl* node), (override));
    MOCK_METHOD(void, visitArrayRef, (const odb::ast::ArrayRef* node), (override));
    MOCK_METHOD(void, visitBinaryOp, (const odb::ast::BinaryOp* node), (override));
    MOCK_METHOD(void, visitBlock, (const odb::ast::Block* node), (override));
    MOCK_METHOD(void, visitCase, (const odb::ast::Case* node), (override));
    MOCK_METHOD(void, visitCaseList, (const odb::ast::CaseList* node), (override));
    MOCK_METHOD(void, visitCommandExpr, (const odb::ast::CommandExpr* node), (override));
    MOCK_METHOD(void, visitCommandStmnt, (const odb::ast::CommandStmnt* node), (override));
    MOCK_METHOD(void, visitConditional, (const odb::ast::Conditional* node), (override));
    MOCK_METHOD(void, visitConstDecl, (const odb::ast::ConstDecl* node), (override));
    MOCK_METHOD(void, visitConstDeclExpr, (const odb::ast::ConstDeclExpr* node), (override));
    MOCK_METHOD(void, visitDefaultCase, (const odb::ast::DefaultCase* node), (override));
    MOCK_METHOD(void, visitExit, (const odb::ast::Exit* node), (override));
    MOCK_METHOD(void, visitForLoop, (const odb::ast::ForLoop* node), (override));
    MOCK_METHOD(void, visitFuncCallExprOrArrayRef, (const odb::ast::FuncCallExprOrArrayRef* node), (override));
    MOCK_METHOD(void, visitFuncCallExpr, (const odb::ast::FuncCallExpr* node), (override));
    MOCK_METHOD(void, visitFuncCallStmnt, (const odb::ast::FuncCallStmnt* node), (override));
    MOCK_METHOD(void, visitFuncDecl, (const odb::ast::FuncDecl* node), (override));
    MOCK_METHOD(void, visitFuncExit, (const odb::ast::FuncExit* node), (override));
    MOCK_METHOD(void, visitGoto, (const odb::ast::Goto* node), (override));
    MOCK_METHOD(void, visitIdentifier, (const odb::ast::Identifier* node), (override));
    MOCK_METHOD(void, visitInfiniteLoop, (const odb::ast::InfiniteLoop* node), (override));
    MOCK_METHOD(void, visitInitializerList, (const odb::ast::InitializerList* node), (override));
    MOCK_METHOD(void, visitLabel, (const odb::ast::Label* node), (override));
    MOCK_METHOD(void, visitScopedIdentifier, (const odb::ast::ScopedIdentifier* node), (override));
    MOCK_METHOD(void, visitSelect, (const odb::ast::Select* node), (override));
    MOCK_METHOD(void, visitSubCall, (const odb::ast::SubCall* node), (override));
    MOCK_METHOD(void, visitSubReturn, (const odb::ast::SubReturn* node), (override));
    MOCK_METHOD(void, visitVarDecl, (const odb::ast::VarDecl* node), (override));
    MOCK_METHOD(void, visitUDTDecl, (const odb::ast::UDTDecl* node), (override));
    MOCK_METHOD(void, visitUDTDeclBody, (const odb::ast::UDTDeclBody* node), (override));
    MOCK_METHOD(void, visitUDTFieldAssignment, (const odb::ast::UDTFieldAssignment* node), (override));
    MOCK_METHOD(void, visitUDTFieldOuter, (const odb::ast::UDTFieldOuter* node), (override));
    MOCK_METHOD(void, visitUDTFieldInner, (const odb::ast::UDTFieldInner* node), (override));
    MOCK_METHOD(void, visitUnaryOp, (const odb::ast::UnaryOp* node), (override));
    MOCK_METHOD(void, visitUntilLoop, (const odb::ast::UntilLoop* node), (override));
    MOCK_METHOD(void, visitVarAssignment, (const odb::ast::VarAssignment* node), (override));
    MOCK_METHOD(void, visitVarRef, (const odb::ast::VarRef* node), (override));
    MOCK_METHOD(void, visitWhileLoop, (const odb::ast::WhileLoop* node), (override));

#define X(dbname, cppname) \
    MOCK_METHOD(void, visit##dbname##Literal, (const odb::ast::dbname##Literal* node), (override));
    ODB_DATATYPE_LIST
#undef X

};
