#pragma once

#include "gmock/gmock.h"
#include "odb-compiler/ast/Visitor.hpp"

class ASTMockVisitor : public odb::ast::Visitor
{
public:
    MOCK_METHOD(void, visitAnnotatedSymbol, (const ast::AnnotatedSymbol* node), (override));
    MOCK_METHOD(void, visitArrayRef, (const ast::ArrayRef* node), (override));
    MOCK_METHOD(void, visitBlock, (const ast::Block* node), (override));
    MOCK_METHOD(void, visitBreak, (const ast::Break* node), (override));
    MOCK_METHOD(void, visitCommandStmntSymbol, (const ast::CommandStmntSymbol* node), (override));
    MOCK_METHOD(void, visitCommandExpr, (const ast::CommandExpr* node), (override));
    MOCK_METHOD(void, visitCommandStmnt, (const ast::CommandStmnt* node), (override));
    MOCK_METHOD(void, visitCommandExprSymbol, (const ast::CommandExprSymbol* node), (override));
    MOCK_METHOD(void, visitConstDecl, (const ast::ConstDecl* node), (override));
    MOCK_METHOD(void, visitDecrementVar, (const ast::DecrementVar* node), (override));
    MOCK_METHOD(void, visitExpressionList, (const ast::ExpressionList* node), (override));
    MOCK_METHOD(void, visitForLoop, (const ast::ForLoop* node), (override));
    MOCK_METHOD(void, visitFuncCallExprOrArrayRef, (const ast::FuncCallExprOrArrayRef* node), (override));
    MOCK_METHOD(void, visitFuncCallExpr, (const ast::FuncCallExpr* node), (override));
    MOCK_METHOD(void, visitFuncCallStmnt, (const ast::FuncCallStmnt* node), (override));
    MOCK_METHOD(void, visitFuncDecl, (const ast::FuncDecl* node), (override));
    MOCK_METHOD(void, visitFuncExit, (const ast::FuncExit* node), (override));
    MOCK_METHOD(void, visitIncrementVar, (const ast::IncrementVar* node), (override));
    MOCK_METHOD(void, visitGoto, (const ast::Goto* node), (override));
    MOCK_METHOD(void, visitGotoSymbol, (const ast::GotoSymbol* node), (override));
    MOCK_METHOD(void, visitInfiniteLoop, (const ast::InfiniteLoop* node), (override));
    MOCK_METHOD(void, visitLabel, (const ast::Label* node), (override));
    MOCK_METHOD(void, visitScopedSymbol, (const ast::ScopedSymbol* node), (override));
    MOCK_METHOD(void, visitScopedAnnotatedSymbol, (const ast::ScopedAnnotatedSymbol* node), (override));
    MOCK_METHOD(void, visitSubCall, (const ast::SubCall* node), (override));
    MOCK_METHOD(void, visitSubCallSymbol, (const ast::SubCallSymbol* node), (override));
    MOCK_METHOD(void, visitSubReturn, (const ast::SubReturn* node), (override));
    MOCK_METHOD(void, visitSymbol, (const ast::Symbol* node), (override));
    MOCK_METHOD(void, visitUntilLoop, (const ast::UntilLoop* node), (override));
    MOCK_METHOD(void, visitVarAssignment, (const ast::VarAssignment* node), (override));
    MOCK_METHOD(void, visitVarRef, (const ast::VarRef* node), (override));
    MOCK_METHOD(void, visitWhileLoop, (const ast::WhileLoop* node), (override));

#define X(dbname, cppname) \
    MOCK_METHOD(void, visit##dbname##Literal, (const ast::dbname##Literal* node), (override)); \
    MOCK_METHOD(void, visit##dbname##VarDecl, (const ast::dbname##VarDecl* node), (override));
    ODB_DATATYPE_LIST
#undef X

#define X(op, tok) \
    MOCK_METHOD(void, visitBinaryOp##op, (const ast::BinaryOp##op* node), (override));
    ODB_BINARY_OP_LIST
#undef X

#define X(op, tok) \
    MOCK_METHOD(void, visitUnaryOp##op, (const ast::UnaryOp##op* node), (override));
    ODB_UNARY_OP_LIST
#undef X
};
