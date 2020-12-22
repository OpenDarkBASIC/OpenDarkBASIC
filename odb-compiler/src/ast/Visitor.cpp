#include "odb-compiler/ast/ArrayRef.hpp"
#include "odb-compiler/ast/Assignment.hpp"
#include "odb-compiler/ast/BinaryOp.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/Break.hpp"
#include "odb-compiler/ast/Command.hpp"
#include "odb-compiler/ast/ConstDecl.hpp"
#include "odb-compiler/ast/Decrement.hpp"
#include "odb-compiler/ast/ExpressionList.hpp"
#include "odb-compiler/ast/FuncCall.hpp"
#include "odb-compiler/ast/FuncDecl.hpp"
#include "odb-compiler/ast/Goto.hpp"
#include "odb-compiler/ast/Increment.hpp"
#include "odb-compiler/ast/Label.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/Loop.hpp"
#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Subroutine.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/UnaryOp.hpp"
#include "odb-compiler/ast/VarDecl.hpp"
#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
void GenericVisitor::visitAnnotatedSymbol(const AnnotatedSymbol* node)               { visit(node); }
void GenericVisitor::visitArrayRef(const ArrayRef* node)                             { visit(node); }
void GenericVisitor::visitBlock(const Block* node)                                   { visit(node); }
void GenericVisitor::visitBreak(const Break* node)                                   { visit(node); }
void GenericVisitor::visitCommandExpr(const CommandExpr* node)                       { visit(node); }
void GenericVisitor::visitCommandExprSymbol(const CommandExprSymbol* node)           { visit(node); }
void GenericVisitor::visitCommandStmntSymbol(const CommandStmntSymbol* node)         { visit(node); }
void GenericVisitor::visitCommandStmnt(const CommandStmnt* node)                     { visit(node); }
void GenericVisitor::visitConstDecl(const ConstDecl* node)                           { visit(node); }
void GenericVisitor::visitDecrementVar(const DecrementVar* node)                     { visit(node); }
void GenericVisitor::visitExpressionList(const ExpressionList* node)                 { visit(node); }
void GenericVisitor::visitForLoop(const ForLoop* node)                               { visit(node); }
void GenericVisitor::visitFuncCallExpr(const FuncCallExpr* node)                     { visit(node); }
void GenericVisitor::visitFuncCallExprOrArrayRef(const FuncCallExprOrArrayRef* node) { visit(node); }
void GenericVisitor::visitFuncCallStmnt(const FuncCallStmnt* node)                   { visit(node); }
void GenericVisitor::visitFuncDecl(const FuncDecl* node)                             { visit(node); }
void GenericVisitor::visitFuncExit(const FuncExit* node)                             { visit(node); }
void GenericVisitor::visitGotoSymbol(const GotoSymbol* node)                         { visit(node); }
void GenericVisitor::visitGoto(const Goto* node)                                     { visit(node); }
void GenericVisitor::visitIncrementVar(const IncrementVar* node)                     { visit(node); }
void GenericVisitor::visitInfiniteLoop(const InfiniteLoop* node)                     { visit(node); }
void GenericVisitor::visitLabel(const Label* node)                                   { visit(node); }
void GenericVisitor::visitScopedSymbol(const ScopedSymbol* node)                     { visit(node); }
void GenericVisitor::visitScopedAnnotatedSymbol(const ScopedAnnotatedSymbol* node)   { visit(node); }
void GenericVisitor::visitSubCall(const SubCall* node)                               { visit(node); }
void GenericVisitor::visitSubCallSymbol(const SubCallSymbol* node)                   { visit(node); }
void GenericVisitor::visitSubReturn(const SubReturn* node)                           { visit(node); }
void GenericVisitor::visitSymbol(const Symbol* node)                                 { visit(node); }
void GenericVisitor::visitUntilLoop(const UntilLoop* node)                           { visit(node); }
void GenericVisitor::visitVarAssignment(const VarAssignment* node)                   { visit(node); }
void GenericVisitor::visitVarRef(const VarRef* node)                                 { visit(node); }
void GenericVisitor::visitWhileLoop(const WhileLoop* node)                           { visit(node); }

#define X(dbname, cppname) \
    void GenericVisitor::visit##dbname##Literal(const dbname##Literal* node)         { visit(node); } \
    void GenericVisitor::visit##dbname##VarDecl(const dbname##VarDecl* node)         { visit(node); }
ODB_DATATYPE_LIST
#undef X

#define X(op, str) \
    void GenericVisitor::visitBinaryOp##op(const BinaryOp##op* node)                 { visit(node); }
ODB_BINARY_OP_LIST
#undef X

#define X(op, str) \
    void GenericVisitor::visitUnaryOp##op(const UnaryOp##op* node)                   { visit(node); }
ODB_UNARY_OP_LIST
#undef X

}
}
