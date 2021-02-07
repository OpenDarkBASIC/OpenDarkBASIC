#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Datatypes.hpp"
#include "odb-compiler/ast/Operators.hpp"

namespace odb {
namespace ast {

class Node;

class AnnotatedSymbol;
class ArrayAssignment;
class ArrayRef;
class Block;
class Break;
class CommandExpr;
class CommandExprSymbol;
class CommandStmnt;
class CommandStmntSymbol;
class Conditional;
class ConstDecl;
class ExpressionList;
class ForLoop;
class FuncCallExpr;
class FuncCallExprOrArrayRef;
class FuncCallStmnt;
class FuncDecl;
class FuncExit;
class Goto;
class GotoSymbol;
class InfiniteLoop;
class Label;
class ScopedSymbol;
class ScopedAnnotatedSymbol;
class SubCallSymbol;
class SubCall;
class SubReturn;
class Symbol;
class UDTArrayDecl;
class UDTArrayDeclSymbol;
class UDTDecl;
class UDTDeclBody;
class UDTRef;
class UDTFieldOuter;
class UDTFieldInner;
class UDTFieldAssignment;
class UDTVarDecl;
class UDTVarDeclSymbol;
class UntilLoop;
class VarAssignment;
class VarRef;
class WhileLoop;

template <typename T> class LiteralTemplate;
template <typename T> class VarDeclTemplate;
template <typename T> class ArrayDeclTemplate;

#define X(dbname, cppname) \
    typedef LiteralTemplate<cppname> dbname##Literal; \
    typedef VarDeclTemplate<cppname> dbname##VarDecl; \
    typedef ArrayDeclTemplate<cppname> dbname##ArrayDecl;
ODB_DATATYPE_LIST
#undef X

#define X(op, str) class BinaryOp##op;
ODB_BINARY_OP_LIST
#undef X

#define X(op, str) class UnaryOp##op;
ODB_UNARY_OP_LIST
#undef X

class Visitor
{
public:
    virtual void visitAnnotatedSymbol(AnnotatedSymbol* node) = 0;
    virtual void visitArrayAssignment(ArrayAssignment* node) = 0;
    virtual void visitArrayRef(ArrayRef* node)  = 0;
    virtual void visitBlock(Block* node) = 0;
    virtual void visitBreak(Break* node) = 0;
    virtual void visitCommandExpr(CommandExpr* node) = 0;
    virtual void visitCommandExprSymbol(CommandExprSymbol* node) = 0;
    virtual void visitCommandStmnt(CommandStmnt* node) = 0;
    virtual void visitCommandStmntSymbol(CommandStmntSymbol* node) = 0;
    virtual void visitConditional(Conditional* node) = 0;
    virtual void visitConstDecl(ConstDecl* node) = 0;
    virtual void visitExpressionList(ExpressionList* node) = 0;
    virtual void visitForLoop(ForLoop* node) = 0;
    virtual void visitFuncCallExpr(FuncCallExpr* node) = 0;
    virtual void visitFuncCallExprOrArrayRef(FuncCallExprOrArrayRef* node)  = 0;
    virtual void visitFuncCallStmnt(FuncCallStmnt* node) = 0;
    virtual void visitFuncDecl(FuncDecl* node) = 0;
    virtual void visitFuncExit(FuncExit* node) = 0;
    virtual void visitGoto(Goto* node) = 0;
    virtual void visitGotoSymbol(GotoSymbol* node) = 0;
    virtual void visitInfiniteLoop(InfiniteLoop* node) = 0;
    virtual void visitLabel(Label* node) = 0;
    virtual void visitScopedSymbol(ScopedSymbol* node) = 0;
    virtual void visitScopedAnnotatedSymbol(ScopedAnnotatedSymbol* node) = 0;
    virtual void visitSubCallSymbol(SubCallSymbol* node) = 0;
    virtual void visitSubCall(SubCall* node) = 0;
    virtual void visitSubReturn(SubReturn* node) = 0;
    virtual void visitSymbol(Symbol* node) = 0;
    virtual void visitUDTArrayDecl(UDTArrayDecl* node) = 0;
    virtual void visitUDTArrayDeclSymbol(UDTArrayDeclSymbol* node) = 0;
    virtual void visitUDTDecl(UDTDecl* node) = 0;
    virtual void visitUDTDeclBody(UDTDeclBody* node) = 0;
    virtual void visitUDTFieldAssignment(UDTFieldAssignment* node) = 0;
    virtual void visitUDTFieldOuter(UDTFieldOuter* node) = 0;
    virtual void visitUDTFieldInner(UDTFieldInner* node) = 0;
    virtual void visitUDTRef(UDTRef* node) = 0;
    virtual void visitUDTVarDecl(UDTVarDecl* node) = 0;
    virtual void visitUDTVarDeclSymbol(UDTVarDeclSymbol* node) = 0;
    virtual void visitUntilLoop(UntilLoop* node) = 0;
    virtual void visitVarAssignment(VarAssignment* node) = 0;
    virtual void visitVarRef(VarRef* node) = 0;
    virtual void visitWhileLoop(WhileLoop* node) = 0;

#define X(dbname, cppname) \
    virtual void visit##dbname##Literal(dbname##Literal* node) = 0; \
    virtual void visit##dbname##VarDecl(dbname##VarDecl* node) = 0; \
    virtual void visit##dbname##ArrayDecl(dbname##ArrayDecl* node) = 0;
    ODB_DATATYPE_LIST
#undef X

#define X(op, str) \
    virtual void visitBinaryOp##op(BinaryOp##op* node) = 0;
    ODB_BINARY_OP_LIST
#undef X

#define X(op, str) \
    virtual void visitUnaryOp##op(UnaryOp##op* node) = 0;
    ODB_UNARY_OP_LIST
#undef X
};

class ConstVisitor
{
public:
    virtual void visitAnnotatedSymbol(const AnnotatedSymbol* node) = 0;
    virtual void visitArrayAssignment(const ArrayAssignment* node) = 0;
    virtual void visitArrayRef(const ArrayRef* node)  = 0;
    virtual void visitBlock(const Block* node) = 0;
    virtual void visitBreak(const Break* node) = 0;
    virtual void visitCommandExpr(const CommandExpr* node) = 0;
    virtual void visitCommandExprSymbol(const CommandExprSymbol* node) = 0;
    virtual void visitCommandStmnt(const CommandStmnt* node) = 0;
    virtual void visitCommandStmntSymbol(const CommandStmntSymbol* node) = 0;
    virtual void visitConditional(const Conditional* node) = 0;
    virtual void visitConstDecl(const ConstDecl* node) = 0;
    virtual void visitExpressionList(const ExpressionList* node) = 0;
    virtual void visitForLoop(const ForLoop* node) = 0;
    virtual void visitFuncCallExpr(const FuncCallExpr* node) = 0;
    virtual void visitFuncCallExprOrArrayRef(const FuncCallExprOrArrayRef* node)  = 0;
    virtual void visitFuncCallStmnt(const FuncCallStmnt* node) = 0;
    virtual void visitFuncDecl(const FuncDecl* node) = 0;
    virtual void visitFuncExit(const FuncExit* node) = 0;
    virtual void visitGoto(const Goto* node) = 0;
    virtual void visitGotoSymbol(const GotoSymbol* node) = 0;
    virtual void visitInfiniteLoop(const InfiniteLoop* node) = 0;
    virtual void visitLabel(const Label* node) = 0;
    virtual void visitScopedSymbol(const ScopedSymbol* node) = 0;
    virtual void visitScopedAnnotatedSymbol(const ScopedAnnotatedSymbol* node) = 0;
    virtual void visitSubCallSymbol(const SubCallSymbol* node) = 0;
    virtual void visitSubCall(const SubCall* node) = 0;
    virtual void visitSubReturn(const SubReturn* node) = 0;
    virtual void visitSymbol(const Symbol* node) = 0;
    virtual void visitUDTArrayDecl(const UDTArrayDecl* node) = 0;
    virtual void visitUDTArrayDeclSymbol(const UDTArrayDeclSymbol* node) = 0;
    virtual void visitUDTDecl(const UDTDecl* node) = 0;
    virtual void visitUDTDeclBody(const UDTDeclBody* node) = 0;
    virtual void visitUDTFieldAssignment(const UDTFieldAssignment* node) = 0;
    virtual void visitUDTFieldOuter(const UDTFieldOuter* node) = 0;
    virtual void visitUDTFieldInner(const UDTFieldInner* node) = 0;
    virtual void visitUDTRef(const UDTRef* node) = 0;
    virtual void visitUDTVarDecl(const UDTVarDecl* node) = 0;
    virtual void visitUDTVarDeclSymbol(const UDTVarDeclSymbol* node) = 0;
    virtual void visitUntilLoop(const UntilLoop* node) = 0;
    virtual void visitVarAssignment(const VarAssignment* node) = 0;
    virtual void visitVarRef(const VarRef* node) = 0;
    virtual void visitWhileLoop(const WhileLoop* node) = 0;

#define X(dbname, cppname) \
    virtual void visit##dbname##Literal(const dbname##Literal* node) = 0; \
    virtual void visit##dbname##VarDecl(const dbname##VarDecl* node) = 0; \
    virtual void visit##dbname##ArrayDecl(const dbname##ArrayDecl* node) = 0;
    ODB_DATATYPE_LIST
#undef X

#define X(op, str) \
    virtual void visitBinaryOp##op(const BinaryOp##op* node) = 0;
    ODB_BINARY_OP_LIST
#undef X

#define X(op, str) \
    virtual void visitUnaryOp##op(const UnaryOp##op* node) = 0;
    ODB_UNARY_OP_LIST
#undef X
};

class GenericVisitor : public Visitor
{
public:
    void visitAnnotatedSymbol(AnnotatedSymbol* node) override;
    void visitArrayAssignment(ArrayAssignment* node) override;
    void visitArrayRef(ArrayRef* node) override;
    void visitBlock(Block* node) override;
    void visitBreak(Break* node) override;
    void visitCommandExpr(CommandExpr* node) override;
    void visitCommandExprSymbol(CommandExprSymbol* node) override;
    void visitCommandStmnt(CommandStmnt* node) override;
    void visitCommandStmntSymbol(CommandStmntSymbol* node) override;
    void visitConditional(Conditional* node) override;
    void visitConstDecl(ConstDecl* node) override;
    void visitExpressionList(ExpressionList* node) override;
    void visitForLoop(ForLoop* node) override;
    void visitFuncCallExpr(FuncCallExpr* node) override;
    void visitFuncCallExprOrArrayRef(FuncCallExprOrArrayRef* node) override;
    void visitFuncCallStmnt(FuncCallStmnt* node) override;
    void visitFuncDecl(FuncDecl* node) override;
    void visitFuncExit(FuncExit* node) override;
    void visitGoto(Goto* node) override;
    void visitGotoSymbol(GotoSymbol* node) override;
    void visitInfiniteLoop(InfiniteLoop* node) override;
    void visitLabel(Label* node) override;
    void visitScopedSymbol(ScopedSymbol* node) override;
    void visitScopedAnnotatedSymbol(ScopedAnnotatedSymbol* node) override;
    void visitSubCallSymbol(SubCallSymbol* node) override;
    void visitSubCall(SubCall* node) override;
    void visitSubReturn(SubReturn* node) override;
    void visitSymbol(Symbol* node) override;
    void visitUDTArrayDecl(UDTArrayDecl* node) override;
    void visitUDTArrayDeclSymbol(UDTArrayDeclSymbol* node) override;
    void visitUDTDecl(UDTDecl* node) override;
    void visitUDTDeclBody(UDTDeclBody* node) override;
    void visitUDTFieldAssignment(UDTFieldAssignment* node) override;
    void visitUDTFieldOuter(UDTFieldOuter* node) override;
    void visitUDTFieldInner(UDTFieldInner* node) override;
    void visitUDTRef(UDTRef* node) override;
    void visitUDTVarDecl(UDTVarDecl* node) override;
    void visitUDTVarDeclSymbol(UDTVarDeclSymbol* node) override;
    void visitUntilLoop(UntilLoop* node) override;
    void visitVarAssignment(VarAssignment* node) override;
    void visitVarRef(VarRef* node) override;
    void visitWhileLoop(WhileLoop* node) override;

#define X(dbname, cppname) \
    void visit##dbname##Literal(dbname##Literal* node) override; \
    void visit##dbname##VarDecl(dbname##VarDecl* node) override; \
    void visit##dbname##ArrayDecl(dbname##ArrayDecl* node) override;
    ODB_DATATYPE_LIST
#undef X

#define X(op, str) \
    void visitBinaryOp##op(BinaryOp##op* node) override;
    ODB_BINARY_OP_LIST
#undef X

#define X(op, str) \
    void visitUnaryOp##op(UnaryOp##op* node) override;
    ODB_UNARY_OP_LIST
#undef X

    virtual void visit(Node* node) = 0;
};

class GenericConstVisitor : public ConstVisitor
{
public:
    void visitAnnotatedSymbol(const AnnotatedSymbol* node) override;
    void visitArrayAssignment(const ArrayAssignment* node) override;
    void visitArrayRef(const ArrayRef* node) override;
    void visitBlock(const Block* node) override;
    void visitBreak(const Break* node) override;
    void visitCommandExpr(const CommandExpr* node) override;
    void visitCommandExprSymbol(const CommandExprSymbol* node) override;
    void visitCommandStmnt(const CommandStmnt* node) override;
    void visitCommandStmntSymbol(const CommandStmntSymbol* node) override;
    void visitConditional(const Conditional* node) override;
    void visitConstDecl(const ConstDecl* node) override;
    void visitExpressionList(const ExpressionList* node) override;
    void visitForLoop(const ForLoop* node) override;
    void visitFuncCallExpr(const FuncCallExpr* node) override;
    void visitFuncCallExprOrArrayRef(const FuncCallExprOrArrayRef* node) override;
    void visitFuncCallStmnt(const FuncCallStmnt* node) override;
    void visitFuncDecl(const FuncDecl* node) override;
    void visitFuncExit(const FuncExit* node) override;
    void visitGoto(const Goto* node) override;
    void visitGotoSymbol(const GotoSymbol* node) override;
    void visitInfiniteLoop(const InfiniteLoop* node) override;
    void visitLabel(const Label* node) override;
    void visitScopedSymbol(const ScopedSymbol* node) override;
    void visitScopedAnnotatedSymbol(const ScopedAnnotatedSymbol* node) override;
    void visitSubCallSymbol(const SubCallSymbol* node) override;
    void visitSubCall(const SubCall* node) override;
    void visitSubReturn(const SubReturn* node) override;
    void visitSymbol(const Symbol* node) override;
    void visitUDTArrayDecl(const UDTArrayDecl* node) override;
    void visitUDTArrayDeclSymbol(const UDTArrayDeclSymbol* node) override;
    void visitUDTDecl(const UDTDecl* node) override;
    void visitUDTDeclBody(const UDTDeclBody* node) override;
    void visitUDTFieldOuter(const UDTFieldOuter* node) override;
    void visitUDTFieldInner(const UDTFieldInner* node) override;
    void visitUDTFieldAssignment(const UDTFieldAssignment* node) override;
    void visitUDTRef(const UDTRef* node) override;
    void visitUDTVarDecl(const UDTVarDecl* node) override;
    void visitUDTVarDeclSymbol(const UDTVarDeclSymbol* node) override;
    void visitUntilLoop(const UntilLoop* node) override;
    void visitVarAssignment(const VarAssignment* node) override;
    void visitVarRef(const VarRef* node) override;
    void visitWhileLoop(const WhileLoop* node) override;

#define X(dbname, cppname) \
    void visit##dbname##Literal(const dbname##Literal* node) override; \
    void visit##dbname##VarDecl(const dbname##VarDecl* node) override; \
    void visit##dbname##ArrayDecl(const dbname##ArrayDecl* node) override;
    ODB_DATATYPE_LIST
#undef X

#define X(op, str) \
    void visitBinaryOp##op(const BinaryOp##op* node) override;
    ODB_BINARY_OP_LIST
#undef X

#define X(op, str) \
    void visitUnaryOp##op(const UnaryOp##op* node) override;
    ODB_UNARY_OP_LIST
#undef X

    virtual void visit(const Node* node) = 0;
};

}
}
