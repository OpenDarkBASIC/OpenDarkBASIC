#include "odb-compiler/tests/ASTParentConsistenciesChecker.hpp"
#include "odb-compiler/ast/Node.hpp"
#include <gmock/gmock.h>

using namespace testing;
using namespace odb;
using namespace ast;

void ASTParentConsistenciesChecker::visitAnnotatedSymbol(const AnnotatedSymbol* node)
{
}
void ASTParentConsistenciesChecker::visitArrayRef(const ArrayRef* node)
{
    EXPECT_THAT(node, Eq(node->symbol()->parent()));
    EXPECT_THAT(node, Eq(node->args()->parent()));
}
void ASTParentConsistenciesChecker::visitBlock(const Block* node)
{
    for (const auto& stmnt : node->statements())
        EXPECT_THAT(node, Eq(stmnt->parent()));
}
void ASTParentConsistenciesChecker::visitConstDecl(const ConstDecl* node)
{
    EXPECT_THAT(node, Eq(node->symbol()->parent()));
    EXPECT_THAT(node, Eq(node->literal()->parent()));
}
void ASTParentConsistenciesChecker::visitExpressionList(const ExpressionList* node)
{
    for (const auto& expr : node->expressions())
        EXPECT_THAT(node, Eq(expr->parent()));
}
void ASTParentConsistenciesChecker::visitFuncCallExpr(const FuncCallExpr* node)
{
    EXPECT_THAT(node, Eq(node->symbol()->parent()));
    if (node->args())
        EXPECT_THAT(node, Eq(node->args()->parent()));
}
void ASTParentConsistenciesChecker::visitFuncCallExprOrArrayRef(const FuncCallExprOrArrayRef* node)
{
    EXPECT_THAT(node, Eq(node->symbol()->parent()));
    EXPECT_THAT(node, Eq(node->args()->parent()));
}
void ASTParentConsistenciesChecker::visitFuncCallStmnt(const FuncCallStmnt* node)
{
    EXPECT_THAT(node, Eq(node->symbol()->parent()));
    if (node->args())
        EXPECT_THAT(node, Eq(node->args()->parent()));
}
void ASTParentConsistenciesChecker::visitKeywordExpr(const KeywordExpr* node)
{
    if (node->args())
        EXPECT_THAT(node, Eq(node->args()->parent()));
}
void ASTParentConsistenciesChecker::visitKeywordExprSymbol(const KeywordExprSymbol* node)
{
    if (node->args())
        EXPECT_THAT(node, Eq(node->args()->parent()));
}
void ASTParentConsistenciesChecker::visitKeywordStmnt(const KeywordStmnt* node)
{
    if (node->args())
        EXPECT_THAT(node, Eq(node->args()->parent()));
}
void ASTParentConsistenciesChecker::visitKeywordStmntSymbol(const KeywordStmntSymbol* node)
{
    if (node->args())
        EXPECT_THAT(node, Eq(node->args()->parent()));
}
void ASTParentConsistenciesChecker::visitScopedSymbol(const ScopedSymbol* node)
{
}
void ASTParentConsistenciesChecker::visitScopedAnnotatedSymbol(const ScopedAnnotatedSymbol* node)
{
}
void ASTParentConsistenciesChecker::visitSymbol(const Symbol* node)
{
}
void ASTParentConsistenciesChecker::visitVarAssignment(const VarAssignment* node)
{
    EXPECT_THAT(node, Eq(node->variable()->parent()));
    EXPECT_THAT(node, Eq(node->expression()->parent()));
}
void ASTParentConsistenciesChecker::visitVarRef(const VarRef* node)
{
    EXPECT_THAT(node, Eq(node->symbol()->parent()));
}

#define X(dbname, cppname) \
    void ASTParentConsistenciesChecker::visit##dbname##Literal(const dbname##Literal* node) {} \
    void ASTParentConsistenciesChecker::visit##dbname##VarDecl(const dbname##VarDecl* node) \
    {                                                                         \
        EXPECT_THAT(node, Eq(node->symbol()->parent()));                      \
        EXPECT_THAT(node, Eq(node->initialValue()->parent()));                \
    }
ODB_DATATYPE_LIST
#undef X
