#include "odb-compiler/ast/ArrayDecl.hpp"
#include "odb-compiler/ast/ArrayRef.hpp"
#include "odb-compiler/ast/Assignment.hpp"
#include "odb-compiler/ast/BinaryOp.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/Break.hpp"
#include "odb-compiler/ast/Command.hpp"
#include "odb-compiler/ast/Conditional.hpp"
#include "odb-compiler/ast/ConstDecl.hpp"
#include "odb-compiler/ast/ExpressionList.hpp"
#include "odb-compiler/ast/FuncCall.hpp"
#include "odb-compiler/ast/FuncDecl.hpp"
#include "odb-compiler/ast/Goto.hpp"
#include "odb-compiler/ast/Label.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/Loop.hpp"
#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Subroutine.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/UDTDecl.hpp"
#include "odb-compiler/ast/UDTField.hpp"
#include "odb-compiler/ast/UDTRef.hpp"
#include "odb-compiler/ast/UnaryOp.hpp"
#include "odb-compiler/ast/VarDecl.hpp"
#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/ast/Visitor.hpp"
#include "odb-compiler/commands/Command.hpp"

namespace odb {
namespace ast {

// ----------------------------------------------------------------------------
void GenericVisitor::visitAnnotatedSymbol(AnnotatedSymbol* node)               { visit(node); }
void GenericVisitor::visitArrayAssignment(ArrayAssignment* node)               { visit(node); }
void GenericVisitor::visitArrayRef(ArrayRef* node)                             { visit(node); }
void GenericVisitor::visitBlock(Block* node)                                   { visit(node); }
void GenericVisitor::visitBreak(Break* node)                                   { visit(node); }
void GenericVisitor::visitCommandExpr(CommandExpr* node)                       { visit(node); }
void GenericVisitor::visitCommandExprSymbol(CommandExprSymbol* node)           { visit(node); }
void GenericVisitor::visitCommandStmntSymbol(CommandStmntSymbol* node)         { visit(node); }
void GenericVisitor::visitCommandStmnt(CommandStmnt* node)                     { visit(node); }
void GenericVisitor::visitConditional(Conditional* node)                       { visit(node); }
void GenericVisitor::visitConstDecl(ConstDecl* node)                           { visit(node); }
void GenericVisitor::visitExpressionList(ExpressionList* node)                 { visit(node); }
void GenericVisitor::visitForLoop(ForLoop* node)                               { visit(node); }
void GenericVisitor::visitFuncCallExpr(FuncCallExpr* node)                     { visit(node); }
void GenericVisitor::visitFuncCallExprOrArrayRef(FuncCallExprOrArrayRef* node) { visit(node); }
void GenericVisitor::visitFuncCallStmnt(FuncCallStmnt* node)                   { visit(node); }
void GenericVisitor::visitFuncDecl(FuncDecl* node)                             { visit(node); }
void GenericVisitor::visitFuncExit(FuncExit* node)                             { visit(node); }
void GenericVisitor::visitGotoSymbol(GotoSymbol* node)                         { visit(node); }
void GenericVisitor::visitGoto(Goto* node)                                     { visit(node); }
void GenericVisitor::visitInfiniteLoop(InfiniteLoop* node)                     { visit(node); }
void GenericVisitor::visitLabel(Label* node)                                   { visit(node); }
void GenericVisitor::visitScopedSymbol(ScopedSymbol* node)                     { visit(node); }
void GenericVisitor::visitScopedAnnotatedSymbol(ScopedAnnotatedSymbol* node)   { visit(node); }
void GenericVisitor::visitSubCall(SubCall* node)                               { visit(node); }
void GenericVisitor::visitSubCallSymbol(SubCallSymbol* node)                   { visit(node); }
void GenericVisitor::visitSubReturn(SubReturn* node)                           { visit(node); }
void GenericVisitor::visitSymbol(Symbol* node)                                 { visit(node); }
void GenericVisitor::visitUDTArrayDecl(UDTArrayDecl* node)                     { visit(node); }
void GenericVisitor::visitUDTArrayDeclSymbol(UDTArrayDeclSymbol* node)         { visit(node); }
void GenericVisitor::visitUDTDecl(UDTDecl* node)                               { visit(node); }
void GenericVisitor::visitUDTDeclBody(UDTDeclBody* node)                       { visit(node); }
void GenericVisitor::visitUDTFieldOuter(UDTFieldOuter* node)                   { visit(node); }
void GenericVisitor::visitUDTFieldInner(UDTFieldInner* node)                   { visit(node); }
void GenericVisitor::visitUDTFieldAssignment(UDTFieldAssignment* node)         { visit(node); }
void GenericVisitor::visitUDTRef(UDTRef* node)                                 { visit(node); }
void GenericVisitor::visitUDTVarDecl(UDTVarDecl* node)                         { visit(node); }
void GenericVisitor::visitUDTVarDeclSymbol(UDTVarDeclSymbol* node)             { visit(node); }
void GenericVisitor::visitUntilLoop(UntilLoop* node)                           { visit(node); }
void GenericVisitor::visitVarAssignment(VarAssignment* node)                   { visit(node); }
void GenericVisitor::visitVarRef(VarRef* node)                                 { visit(node); }
void GenericVisitor::visitWhileLoop(WhileLoop* node)                           { visit(node); }

#define X(dbname, cppname) \
    void GenericVisitor::visit##dbname##Literal(dbname##Literal* node)         { visit(node); } \
    void GenericVisitor::visit##dbname##VarDecl(dbname##VarDecl* node)         { visit(node); } \
    void GenericVisitor::visit##dbname##ArrayDecl(dbname##ArrayDecl* node)     { visit(node); }
ODB_DATATYPE_LIST
#undef X

#define X(op, str) \
    void GenericVisitor::visitBinaryOp##op(BinaryOp##op* node)                 { visit(node); }
ODB_BINARY_OP_LIST
#undef X

#define X(op, str) \
    void GenericVisitor::visitUnaryOp##op(UnaryOp##op* node)                   { visit(node); }
ODB_UNARY_OP_LIST
#undef X

// ----------------------------------------------------------------------------
void GenericConstVisitor::visitAnnotatedSymbol(const AnnotatedSymbol* node)               { visit(node); }
void GenericConstVisitor::visitArrayAssignment(const ArrayAssignment* node)               { visit(node); }
void GenericConstVisitor::visitArrayRef(const ArrayRef* node)                             { visit(node); }
void GenericConstVisitor::visitBlock(const Block* node)                                   { visit(node); }
void GenericConstVisitor::visitBreak(const Break* node)                                   { visit(node); }
void GenericConstVisitor::visitCommandExpr(const CommandExpr* node)                       { visit(node); }
void GenericConstVisitor::visitCommandExprSymbol(const CommandExprSymbol* node)           { visit(node); }
void GenericConstVisitor::visitCommandStmntSymbol(const CommandStmntSymbol* node)         { visit(node); }
void GenericConstVisitor::visitCommandStmnt(const CommandStmnt* node)                     { visit(node); }
void GenericConstVisitor::visitConditional(const Conditional* node)                       { visit(node); }
void GenericConstVisitor::visitConstDecl(const ConstDecl* node)                           { visit(node); }
void GenericConstVisitor::visitExpressionList(const ExpressionList* node)                 { visit(node); }
void GenericConstVisitor::visitForLoop(const ForLoop* node)                               { visit(node); }
void GenericConstVisitor::visitFuncCallExpr(const FuncCallExpr* node)                     { visit(node); }
void GenericConstVisitor::visitFuncCallExprOrArrayRef(const FuncCallExprOrArrayRef* node) { visit(node); }
void GenericConstVisitor::visitFuncCallStmnt(const FuncCallStmnt* node)                   { visit(node); }
void GenericConstVisitor::visitFuncDecl(const FuncDecl* node)                             { visit(node); }
void GenericConstVisitor::visitFuncExit(const FuncExit* node)                             { visit(node); }
void GenericConstVisitor::visitGotoSymbol(const GotoSymbol* node)                         { visit(node); }
void GenericConstVisitor::visitGoto(const Goto* node)                                     { visit(node); }
void GenericConstVisitor::visitInfiniteLoop(const InfiniteLoop* node)                     { visit(node); }
void GenericConstVisitor::visitLabel(const Label* node)                                   { visit(node); }
void GenericConstVisitor::visitScopedSymbol(const ScopedSymbol* node)                     { visit(node); }
void GenericConstVisitor::visitScopedAnnotatedSymbol(const ScopedAnnotatedSymbol* node)   { visit(node); }
void GenericConstVisitor::visitSubCall(const SubCall* node)                               { visit(node); }
void GenericConstVisitor::visitSubCallSymbol(const SubCallSymbol* node)                   { visit(node); }
void GenericConstVisitor::visitSubReturn(const SubReturn* node)                           { visit(node); }
void GenericConstVisitor::visitSymbol(const Symbol* node)                                 { visit(node); }
void GenericConstVisitor::visitUDTArrayDecl(const UDTArrayDecl* node)                     { visit(node); }
void GenericConstVisitor::visitUDTArrayDeclSymbol(const UDTArrayDeclSymbol* node)         { visit(node); }
void GenericConstVisitor::visitUDTDecl(const UDTDecl* node)                               { visit(node); }
void GenericConstVisitor::visitUDTDeclBody(const UDTDeclBody* node)                       { visit(node); }
void GenericConstVisitor::visitUDTFieldOuter(const UDTFieldOuter* node)                   { visit(node); }
void GenericConstVisitor::visitUDTFieldInner(const UDTFieldInner* node)                   { visit(node); }
void GenericConstVisitor::visitUDTFieldAssignment(const UDTFieldAssignment* node)         { visit(node); }
void GenericConstVisitor::visitUDTRef(const UDTRef* node)                                 { visit(node); }
void GenericConstVisitor::visitUDTVarDecl(const UDTVarDecl* node)                         { visit(node); }
void GenericConstVisitor::visitUDTVarDeclSymbol(const UDTVarDeclSymbol* node)             { visit(node); }
void GenericConstVisitor::visitUntilLoop(const UntilLoop* node)                           { visit(node); }
void GenericConstVisitor::visitVarAssignment(const VarAssignment* node)                   { visit(node); }
void GenericConstVisitor::visitVarRef(const VarRef* node)                                 { visit(node); }
void GenericConstVisitor::visitWhileLoop(const WhileLoop* node)                           { visit(node); }

#define X(dbname, cppname) \
    void GenericConstVisitor::visit##dbname##Literal(const dbname##Literal* node)         { visit(node); } \
    void GenericConstVisitor::visit##dbname##VarDecl(const dbname##VarDecl* node)         { visit(node); } \
    void GenericConstVisitor::visit##dbname##ArrayDecl(const dbname##ArrayDecl* node)     { visit(node); }
ODB_DATATYPE_LIST
#undef X

#define X(op, str) \
    void GenericConstVisitor::visitBinaryOp##op(const BinaryOp##op* node)                 { visit(node); }
ODB_BINARY_OP_LIST
#undef X

#define X(op, str) \
    void GenericConstVisitor::visitUnaryOp##op(const UnaryOp##op* node)                   { visit(node); }
ODB_UNARY_OP_LIST
#undef X

}
}
