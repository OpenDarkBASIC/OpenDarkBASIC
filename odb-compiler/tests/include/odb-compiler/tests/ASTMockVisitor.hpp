#pragma once

#include "gmock/gmock.h"
#include "odb-compiler/ast/Visitor.hpp"

class ASTMockVisitor : public odb::ast::ConstVisitor
{
public:
    MOCK_METHOD(void, visitAnnotatedSymbol, (const odb::ast::AnnotatedSymbol* node), (override));
    MOCK_METHOD(void, visitArrayRef, (const odb::ast::ArrayRef* node), (override));
    MOCK_METHOD(void, visitBlock, (const odb::ast::Block* node), (override));
    MOCK_METHOD(void, visitBreak, (const odb::ast::Break* node), (override));
    MOCK_METHOD(void, visitCommandStmntSymbol, (const odb::ast::CommandStmntSymbol* node), (override));
    MOCK_METHOD(void, visitCommandExpr, (const odb::ast::CommandExpr* node), (override));
    MOCK_METHOD(void, visitCommandStmnt, (const odb::ast::CommandStmnt* node), (override));
    MOCK_METHOD(void, visitCommandExprSymbol, (const odb::ast::CommandExprSymbol* node), (override));
    MOCK_METHOD(void, visitConditional, (const odb::ast::Conditional* node), (override));
    MOCK_METHOD(void, visitConstDecl, (const odb::ast::ConstDecl* node), (override));
    MOCK_METHOD(void, visitDecrementVar, (const odb::ast::DecrementVar* node), (override));
    MOCK_METHOD(void, visitExpressionList, (const odb::ast::ExpressionList* node), (override));
    MOCK_METHOD(void, visitForLoop, (const odb::ast::ForLoop* node), (override));
    MOCK_METHOD(void, visitFuncCallExprOrArrayRef, (const odb::ast::FuncCallExprOrArrayRef* node), (override));
    MOCK_METHOD(void, visitFuncCallExpr, (const odb::ast::FuncCallExpr* node), (override));
    MOCK_METHOD(void, visitFuncCallStmnt, (const odb::ast::FuncCallStmnt* node), (override));
    MOCK_METHOD(void, visitFuncDecl, (const odb::ast::FuncDecl* node), (override));
    MOCK_METHOD(void, visitFuncExit, (const odb::ast::FuncExit* node), (override));
    MOCK_METHOD(void, visitIncrementVar, (const odb::ast::IncrementVar* node), (override));
    MOCK_METHOD(void, visitGoto, (const odb::ast::Goto* node), (override));
    MOCK_METHOD(void, visitGotoSymbol, (const odb::ast::GotoSymbol* node), (override));
    MOCK_METHOD(void, visitInfiniteLoop, (const odb::ast::InfiniteLoop* node), (override));
    MOCK_METHOD(void, visitLabel, (const odb::ast::Label* node), (override));
    MOCK_METHOD(void, visitScopedSymbol, (const odb::ast::ScopedSymbol* node), (override));
    MOCK_METHOD(void, visitScopedAnnotatedSymbol, (const odb::ast::ScopedAnnotatedSymbol* node), (override));
    MOCK_METHOD(void, visitSubCall, (const odb::ast::SubCall* node), (override));
    MOCK_METHOD(void, visitSubCallSymbol, (const odb::ast::SubCallSymbol* node), (override));
    MOCK_METHOD(void, visitSubReturn, (const odb::ast::SubReturn* node), (override));
    MOCK_METHOD(void, visitSymbol, (const odb::ast::Symbol* node), (override));
    MOCK_METHOD(void, visitUntilLoop, (const odb::ast::UntilLoop* node), (override));
    MOCK_METHOD(void, visitVarAssignment, (const odb::ast::VarAssignment* node), (override));
    MOCK_METHOD(void, visitVarRef, (const odb::ast::VarRef* node), (override));
    MOCK_METHOD(void, visitWhileLoop, (const odb::ast::WhileLoop* node), (override));

#define X(dbname, cppname) \
    MOCK_METHOD(void, visit##dbname##Literal, (const odb::ast::dbname##Literal* node), (override)); \
    MOCK_METHOD(void, visit##dbname##VarDecl, (const odb::ast::dbname##VarDecl* node), (override));
    ODB_DATATYPE_LIST
#undef X

#define X(op, tok) \
    MOCK_METHOD(void, visitBinaryOp##op, (const odb::ast::BinaryOp##op* node), (override));
    ODB_BINARY_OP_LIST
#undef X

#define X(op, tok) \
    MOCK_METHOD(void, visitUnaryOp##op, (const odb::ast::UnaryOp##op* node), (override));
    ODB_UNARY_OP_LIST
#undef X
};
