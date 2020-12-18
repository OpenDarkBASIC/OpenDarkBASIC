#pragma once

#include "gmock/gmock.h"
#include "odb-compiler/ast/Visitor.hpp"

class ASTMockVisitor : public odb::ast::Visitor
{
public:
    MOCK_METHOD(void, visitAnnotatedSymbol, (const ast::AnnotatedSymbol* node), (override));
    MOCK_METHOD(void, visitArrayRef, (const ast::ArrayRef* node), (override));
    MOCK_METHOD(void, visitBlock, (const ast::Block* node), (override));
    MOCK_METHOD(void, visitConstDecl, (const ast::ConstDecl* node), (override));
    MOCK_METHOD(void, visitExpressionList, (const ast::ExpressionList* node), (override));
    MOCK_METHOD(void, visitForLoop, (const ast::ForLoop* node), (override));
    MOCK_METHOD(void, visitFuncCallExprOrArrayRef, (const ast::FuncCallExprOrArrayRef* node), (override));
    MOCK_METHOD(void, visitFuncCallExpr, (const ast::FuncCallExpr* node), (override));
    MOCK_METHOD(void, visitKeywordExprSymbol, (const ast::KeywordExprSymbol* node), (override));
    MOCK_METHOD(void, visitFuncCallStmnt, (const ast::FuncCallStmnt* node), (override));
    MOCK_METHOD(void, visitInfiniteLoop, (const ast::InfiniteLoop* node), (override));
    MOCK_METHOD(void, visitKeywordStmntSymbol, (const ast::KeywordStmntSymbol* node), (override));
    MOCK_METHOD(void, visitKeywordExpr, (const ast::KeywordExpr* node), (override));
    MOCK_METHOD(void, visitKeywordStmnt, (const ast::KeywordStmnt* node), (override));
    MOCK_METHOD(void, visitScopedSymbol, (const ast::ScopedSymbol* node), (override));
    MOCK_METHOD(void, visitScopedAnnotatedSymbol, (const ast::ScopedAnnotatedSymbol* node), (override));
    MOCK_METHOD(void, visitSymbol, (const ast::Symbol* node), (override));
    MOCK_METHOD(void, visitUntilLoop, (const ast::UntilLoop* node), (override));
    MOCK_METHOD(void, visitVarAssignment, (const ast::VarAssignment* node), (override));
    MOCK_METHOD(void, visitVarRef, (const ast::VarRef* node), (override));
    MOCK_METHOD(void, visitWhileLoop, (const ast::WhileLoop* node), (override));

#define X(dbname, cppname) \
    MOCK_METHOD(void, visit##dbname##Literal, (const ast::dbname##Literal* node), (override));
    ODB_DATATYPE_LIST
#undef X

#define X(dbname, cppname) \
    MOCK_METHOD(void, visit##dbname##VarDecl, (const ast::dbname##VarDecl* node), (override));
    ODB_DATATYPE_LIST
#undef X
};
