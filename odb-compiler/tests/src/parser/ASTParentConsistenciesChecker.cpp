#include "odb-compiler/tests/ASTParentConsistenciesChecker.hpp"
#include "odb-compiler/ast/ArrayDecl.hpp"
#include "odb-compiler/ast/ArrayRef.hpp"
#include "odb-compiler/ast/Assignment.hpp"
#include "odb-compiler/ast/BinaryOp.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/Command.hpp"
#include "odb-compiler/ast/Conditional.hpp"
#include "odb-compiler/ast/ConstDecl.hpp"
#include "odb-compiler/ast/Exit.hpp"
#include "odb-compiler/ast/FuncCall.hpp"
#include "odb-compiler/ast/FuncDecl.hpp"
#include "odb-compiler/ast/Goto.hpp"
#include "odb-compiler/ast/Label.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/Loop.hpp"
#include "odb-compiler/ast/SelectCase.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Subroutine.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/UDTDecl.hpp"
#include "odb-compiler/ast/UDTField.hpp"
#include "odb-compiler/ast/UDTRef.hpp"
#include "odb-compiler/ast/UnaryOp.hpp"
#include "odb-compiler/ast/VarDecl.hpp"
#include "odb-compiler/ast/VarRef.hpp"
#include <gmock/gmock.h>

using namespace testing;
using namespace odb;
using namespace ast;

void ASTParentConsistenciesChecker::visitAnnotatedSymbol(const AnnotatedSymbol* node) {}
void ASTParentConsistenciesChecker::visitArrayAssignment(const ArrayAssignment* node)
{
    EXPECT_THAT(node, Eq(node->array()->parent()));
    EXPECT_THAT(node, Eq(node->expression()->parent()));
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
void ASTParentConsistenciesChecker::visitCase(const Case* node)
{
    EXPECT_THAT(node, Eq(node->expression()->parent()));
    if (node->body().notNull())
        EXPECT_THAT(node, Eq(node->body()->parent()));
}
void ASTParentConsistenciesChecker::visitCaseList(const CaseList* node)
{
    for (const auto& case_ : node->cases())
        EXPECT_THAT(node, Eq(case_->parent()));
    if (node->defaultCase().notNull())
        EXPECT_THAT(node, Eq(node->defaultCase()->parent()));
}
void ASTParentConsistenciesChecker::visitCommandExpr(const CommandExpr* node)
{
    if (node->args().notNull())
        EXPECT_THAT(node, Eq(node->args()->parent()));
}
void ASTParentConsistenciesChecker::visitCommandExprSymbol(const CommandExprSymbol* node)
{
    if (node->args().notNull())
        EXPECT_THAT(node, Eq(node->args()->parent()));
}
void ASTParentConsistenciesChecker::visitCommandStmnt(const CommandStmnt* node)
{
    if (node->args().notNull())
        EXPECT_THAT(node, Eq(node->args()->parent()));
}
void ASTParentConsistenciesChecker::visitCommandStmntSymbol(const CommandStmntSymbol* node)
{
    if (node->args().notNull())
        EXPECT_THAT(node, Eq(node->args()->parent()));
}
void ASTParentConsistenciesChecker::visitConditional(const Conditional* node)
{
    EXPECT_THAT(node, Eq(node->condition()->parent()));
    if (node->trueBranch().notNull())
        EXPECT_THAT(node, Eq(node->trueBranch()->parent()));
    if (node->falseBranch().notNull())
        EXPECT_THAT(node, Eq(node->falseBranch()->parent()));
}
void ASTParentConsistenciesChecker::visitConstDecl(const ConstDecl* node)
{
    EXPECT_THAT(node, Eq(node->symbol()->parent()));
    EXPECT_THAT(node, Eq(node->literal()->parent()));
}
void ASTParentConsistenciesChecker::visitDefaultCase(const DefaultCase* node)
{
    if (node->body().notNull())
        EXPECT_THAT(node, Eq(node->body()->parent()));
}
void ASTParentConsistenciesChecker::visitExit(const Exit* node) {}
void ASTParentConsistenciesChecker::visitExpressionList(const ExpressionList* node)
{
    for (const auto& expr : node->expressions())
        EXPECT_THAT(node, Eq(expr->parent()));
}
void ASTParentConsistenciesChecker::visitForLoop(const ForLoop* node)
{
    EXPECT_THAT(node, Eq(node->counter()->parent()));
    EXPECT_THAT(node, Eq(node->endValue()->parent()));
    if (node->stepValue().notNull())
        EXPECT_THAT(node, Eq(node->stepValue()->parent()));
    if (node->nextSymbol().notNull())
        EXPECT_THAT(node, Eq(node->nextSymbol()->parent()));
    if (node->body().notNull())
        EXPECT_THAT(node, Eq(node->body()->parent()));
}
void ASTParentConsistenciesChecker::visitFuncCallExpr(const FuncCallExpr* node)
{
    EXPECT_THAT(node, Eq(node->symbol()->parent()));
    if (node->args().notNull())
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
    if (node->args().notNull())
        EXPECT_THAT(node, Eq(node->args()->parent()));
}
void ASTParentConsistenciesChecker::visitFuncDecl(const FuncDecl* node)
{
    EXPECT_THAT(node, Eq(node->symbol()->parent()));
    if (node->args().notNull())
        EXPECT_THAT(node, Eq(node->args()->parent()));
    if (node->body().notNull())
        EXPECT_THAT(node, Eq(node->body()->parent()));
    if (node->returnValue().notNull())
        EXPECT_THAT(node, Eq(node->returnValue()->parent()));
}
void ASTParentConsistenciesChecker::visitFuncExit(const FuncExit* node)
{
    if (node->returnValue().notNull())
        EXPECT_THAT(node, Eq(node->returnValue()->parent()));
}
void ASTParentConsistenciesChecker::visitGoto(const Goto* node)
{
    EXPECT_THAT(node, Eq(node->label()->parent()));
}
void ASTParentConsistenciesChecker::visitGotoSymbol(const GotoSymbol* node)
{
    EXPECT_THAT(node, Eq(node->labelSymbol()->parent()));
}
void ASTParentConsistenciesChecker::visitInfiniteLoop(const InfiniteLoop* node)
{
    if (node->body().notNull())
        EXPECT_THAT(node, Eq(node->body()->parent()));
}
void ASTParentConsistenciesChecker::visitLabel(const Label* node)
{
    EXPECT_THAT(node, Eq(node->symbol()->parent()));
}
void ASTParentConsistenciesChecker::visitScopedSymbol(const ScopedSymbol* node) {}
void ASTParentConsistenciesChecker::visitScopedAnnotatedSymbol(const ScopedAnnotatedSymbol* node) {}
void ASTParentConsistenciesChecker::visitSelect(const Select* node)
{
    EXPECT_THAT(node, Eq(node->expression()->parent()));
    if (node->cases().notNull())
        EXPECT_THAT(node, Eq(node->cases()->parent()));
}
void ASTParentConsistenciesChecker::visitSubCall(const SubCall* node)
{
    EXPECT_THAT(node, Eq(node->label()->parent()));
}
void ASTParentConsistenciesChecker::visitSubCallSymbol(const SubCallSymbol* node)
{
    EXPECT_THAT(node, Eq(node->labelSymbol()->parent()));
}
void ASTParentConsistenciesChecker::visitSubReturn(const SubReturn* node) {}
void ASTParentConsistenciesChecker::visitSymbol(const Symbol* node) {}
void ASTParentConsistenciesChecker::visitUDTArrayDecl(const UDTArrayDecl* node)
{
    EXPECT_THAT(node, Eq(node->symbol()->parent()));
    EXPECT_THAT(node, Eq(node->dims()->parent()));
    EXPECT_THAT(node, Eq(node->udt()->parent()));
}
void ASTParentConsistenciesChecker::visitUDTArrayDeclSymbol(const UDTArrayDeclSymbol* node)
{
    EXPECT_THAT(node, Eq(node->symbol()->parent()));
    EXPECT_THAT(node, Eq(node->dims()->parent()));
    EXPECT_THAT(node, Eq(node->udtSymbol()->parent()));
}
void ASTParentConsistenciesChecker::visitUDTDecl(const UDTDecl* node)
{
    EXPECT_THAT(node, Eq(node->typeName()->parent()));
    EXPECT_THAT(node, Eq(node->body()->parent()));
}
void ASTParentConsistenciesChecker::visitUDTDeclBody(const UDTDeclBody* node)
{
    for (const auto& decl : node->varDeclarations())
        EXPECT_THAT(node, Eq(decl->parent()));
    for (const auto& decl : node->arrayDeclarations())
        EXPECT_THAT(node, Eq(decl->parent()));
}
void ASTParentConsistenciesChecker::visitUDTFieldAssignment(const UDTFieldAssignment* node)
{
    EXPECT_THAT(node, Eq(node->field()->parent()));
    EXPECT_THAT(node, Eq(node->expression()->parent()));
}
void ASTParentConsistenciesChecker::visitUDTFieldOuter(const UDTFieldOuter* node)
{
    EXPECT_THAT(node, Eq(node->left()->parent()));
    EXPECT_THAT(node, Eq(node->right()->parent()));
}
void ASTParentConsistenciesChecker::visitUDTFieldInner(const UDTFieldInner* node)
{
    EXPECT_THAT(node, Eq(node->left()->parent()));
    EXPECT_THAT(node, Eq(node->right()->parent()));
}
void ASTParentConsistenciesChecker::visitUDTRef(const UDTRef* node) {}
void ASTParentConsistenciesChecker::visitUDTVarDecl(const UDTVarDecl* node)
{
    EXPECT_THAT(node, Eq(node->symbol()->parent()));
    EXPECT_THAT(node, Eq(node->udt()->parent()));
}
void ASTParentConsistenciesChecker::visitUDTVarDeclSymbol(const UDTVarDeclSymbol* node)
{
    EXPECT_THAT(node, Eq(node->symbol()->parent()));
    EXPECT_THAT(node, Eq(node->udtSymbol()->parent()));
}
void ASTParentConsistenciesChecker::visitUntilLoop(const UntilLoop* node)
{
    EXPECT_THAT(node, Eq(node->exitCondition()->parent()));
    if (node->body().notNull())
        EXPECT_THAT(node, Eq(node->body()->parent()));
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
void ASTParentConsistenciesChecker::visitWhileLoop(const WhileLoop* node)
{
    EXPECT_THAT(node, Eq(node->continueCondition()->parent()));
    if (node->body().notNull())
        EXPECT_THAT(node, Eq(node->body()->parent()));
}

#define X(dbname, cppname) \
    void ASTParentConsistenciesChecker::visit##dbname##Literal(const dbname##Literal* node) {} \
    void ASTParentConsistenciesChecker::visit##dbname##VarDecl(const dbname##VarDecl* node) \
    {                                                                         \
        EXPECT_THAT(node, Eq(node->symbol()->parent()));                      \
        EXPECT_THAT(node, Eq(node->initialValue()->parent()));                \
    }                                                                         \
    void ASTParentConsistenciesChecker::visit##dbname##ArrayDecl(const dbname##ArrayDecl* node) \
    {                                                                         \
        EXPECT_THAT(node, Eq(node->symbol()->parent()));                      \
        EXPECT_THAT(node, Eq(node->dims()->parent()));                        \
    }
ODB_DATATYPE_LIST
#undef X

#define X(op, tok)                                                            \
    void ASTParentConsistenciesChecker::visitBinaryOp##op(const BinaryOp##op* node) \
    {                                                                         \
        EXPECT_THAT(node, Eq(node->lhs()->parent()));                         \
        EXPECT_THAT(node, Eq(node->rhs()->parent()));                         \
    }
ODB_BINARY_OP_LIST
#undef X

#define X(op, tok)                                                            \
    void ASTParentConsistenciesChecker::visitUnaryOp##op(const UnaryOp##op* node) \
    {                                                                         \
        EXPECT_THAT(node, Eq(node->expr()->parent()));                        \
    }
ODB_UNARY_OP_LIST
#undef X
