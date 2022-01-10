#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Datatypes.hpp"

namespace odb::ast {

class Node;

class AnnotatedSymbol;
class ArgList;
class ArrayAssignment;
class ArrayRef;
class BinaryOp;
class Block;
class Case;
class CaseList;
class Exit;
class CommandExpr;
class CommandStmnt;
class Conditional;
class ConstDecl;
class ConstDeclExpr;
class DefaultCase;
class ForLoop;
class FuncCallExpr;
class FuncCallExprOrArrayRef;
class FuncCallStmnt;
class FuncDecl;
class FuncExit;
class Goto;
class InfiniteLoop;
class InitializerList;
class Label;
class ScopedAnnotatedSymbol;
class Select;
class SubCall;
class SubReturn;
class Symbol;
class UDTArrayDecl;
class UDTDecl;
class UDTDeclBody;
class UDTRef;
class UDTFieldOuter;
class UDTFieldInner;
class UDTFieldAssignment;
class UDTVarDecl;
class UnaryOp;
class UntilLoop;
class VarAssignment;
class VarRef;
class WhileLoop;

#define X(dbname, cppname) \
    class dbname##Literal; \
    class dbname##VarDecl; \
    class dbname##ArrayDecl;
ODB_DATATYPE_LIST
#undef X

class Visitor
{
public:
    virtual ~Visitor() = default;

    virtual void visitAnnotatedSymbol(AnnotatedSymbol* node) = 0;
    virtual void visitArgList(ArgList* node) = 0;
    virtual void visitArrayAssignment(ArrayAssignment* node) = 0;
    virtual void visitArrayRef(ArrayRef* node) = 0;
    virtual void visitBinaryOp(BinaryOp* node) = 0;
    virtual void visitBlock(Block* node) = 0;
    virtual void visitCase(Case* node) = 0;
    virtual void visitCaseList(CaseList* node) = 0;
    virtual void visitCommandExpr(CommandExpr* node) = 0;
    virtual void visitCommandStmnt(CommandStmnt* node) = 0;
    virtual void visitConditional(Conditional* node) = 0;
    virtual void visitConstDecl(ConstDecl* node) = 0;
    virtual void visitConstDeclExpr(ConstDeclExpr* node) = 0;
    virtual void visitDefaultCase(DefaultCase* node) = 0;
    virtual void visitExit(Exit* node) = 0;
    virtual void visitForLoop(ForLoop* node) = 0;
    virtual void visitFuncCallExpr(FuncCallExpr* node) = 0;
    virtual void visitFuncCallExprOrArrayRef(FuncCallExprOrArrayRef* node)  = 0;
    virtual void visitFuncCallStmnt(FuncCallStmnt* node) = 0;
    virtual void visitFuncDecl(FuncDecl* node) = 0;
    virtual void visitFuncExit(FuncExit* node) = 0;
    virtual void visitGoto(Goto* node) = 0;
    virtual void visitInfiniteLoop(InfiniteLoop* node) = 0;
    virtual void visitInitializerList(InitializerList* node) = 0;
    virtual void visitLabel(Label* node) = 0;
    virtual void visitScopedAnnotatedSymbol(ScopedAnnotatedSymbol* node) = 0;
    virtual void visitSelect(Select* node) = 0;
    virtual void visitSubCall(SubCall* node) = 0;
    virtual void visitSubReturn(SubReturn* node) = 0;
    virtual void visitSymbol(Symbol* node) = 0;
    virtual void visitUDTArrayDecl(UDTArrayDecl* node) = 0;
    virtual void visitUDTDecl(UDTDecl* node) = 0;
    virtual void visitUDTDeclBody(UDTDeclBody* node) = 0;
    virtual void visitUDTFieldAssignment(UDTFieldAssignment* node) = 0;
    virtual void visitUDTFieldOuter(UDTFieldOuter* node) = 0;
    virtual void visitUDTFieldInner(UDTFieldInner* node) = 0;
    virtual void visitUDTRef(UDTRef* node) = 0;
    virtual void visitUDTVarDecl(UDTVarDecl* node) = 0;
    virtual void visitUnaryOp(UnaryOp* node) = 0;
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
};

class ConstVisitor
{
public:
    virtual ~ConstVisitor() = default;

    virtual void visitAnnotatedSymbol(const AnnotatedSymbol* node) = 0;
    virtual void visitArgList(const ArgList* node) = 0;
    virtual void visitArrayAssignment(const ArrayAssignment* node) = 0;
    virtual void visitArrayRef(const ArrayRef* node)  = 0;
    virtual void visitBinaryOp(const BinaryOp* node) = 0;
    virtual void visitBlock(const Block* node) = 0;
    virtual void visitCase(const Case* node) = 0;
    virtual void visitCaseList(const CaseList* node) = 0;
    virtual void visitCommandExpr(const CommandExpr* node) = 0;
    virtual void visitCommandStmnt(const CommandStmnt* node) = 0;
    virtual void visitConditional(const Conditional* node) = 0;
    virtual void visitConstDecl(const ConstDecl* node) = 0;
    virtual void visitConstDeclExpr(const ConstDeclExpr* node) = 0;
    virtual void visitDefaultCase(const DefaultCase* node) = 0;
    virtual void visitExit(const Exit* node) = 0;
    virtual void visitForLoop(const ForLoop* node) = 0;
    virtual void visitFuncCallExpr(const FuncCallExpr* node) = 0;
    virtual void visitFuncCallExprOrArrayRef(const FuncCallExprOrArrayRef* node)  = 0;
    virtual void visitFuncCallStmnt(const FuncCallStmnt* node) = 0;
    virtual void visitFuncDecl(const FuncDecl* node) = 0;
    virtual void visitFuncExit(const FuncExit* node) = 0;
    virtual void visitGoto(const Goto* node) = 0;
    virtual void visitInfiniteLoop(const InfiniteLoop* node) = 0;
    virtual void visitInitializerList(const InitializerList* node) = 0;
    virtual void visitLabel(const Label* node) = 0;
    virtual void visitScopedAnnotatedSymbol(const ScopedAnnotatedSymbol* node) = 0;
    virtual void visitSelect(const Select* node) = 0;
    virtual void visitSubCall(const SubCall* node) = 0;
    virtual void visitSubReturn(const SubReturn* node) = 0;
    virtual void visitSymbol(const Symbol* node) = 0;
    virtual void visitUDTArrayDecl(const UDTArrayDecl* node) = 0;
    virtual void visitUDTDecl(const UDTDecl* node) = 0;
    virtual void visitUDTDeclBody(const UDTDeclBody* node) = 0;
    virtual void visitUDTFieldAssignment(const UDTFieldAssignment* node) = 0;
    virtual void visitUDTFieldOuter(const UDTFieldOuter* node) = 0;
    virtual void visitUDTFieldInner(const UDTFieldInner* node) = 0;
    virtual void visitUDTRef(const UDTRef* node) = 0;
    virtual void visitUDTVarDecl(const UDTVarDecl* node) = 0;
    virtual void visitUnaryOp(const UnaryOp* node) = 0;
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
};

class GenericVisitor : public Visitor
{
public:
    void visitAnnotatedSymbol(AnnotatedSymbol* node) override;
    void visitArgList(ArgList* node) override;
    void visitArrayAssignment(ArrayAssignment* node) override;
    void visitArrayRef(ArrayRef* node) override;
    void visitBinaryOp(BinaryOp* node) override;
    void visitBlock(Block* node) override;
    void visitCase(Case* node) override;
    void visitCaseList(CaseList* node) override;
    void visitCommandExpr(CommandExpr* node) override;
    void visitCommandStmnt(CommandStmnt* node) override;
    void visitConditional(Conditional* node) override;
    void visitConstDecl(ConstDecl* node) override;
    void visitConstDeclExpr(ConstDeclExpr* node) override;
    void visitDefaultCase(DefaultCase* node) override;
    void visitExit(Exit* node) override;
    void visitForLoop(ForLoop* node) override;
    void visitFuncCallExpr(FuncCallExpr* node) override;
    void visitFuncCallExprOrArrayRef(FuncCallExprOrArrayRef* node) override;
    void visitFuncCallStmnt(FuncCallStmnt* node) override;
    void visitFuncDecl(FuncDecl* node) override;
    void visitFuncExit(FuncExit* node) override;
    void visitGoto(Goto* node) override;
    void visitInfiniteLoop(InfiniteLoop* node) override;
    void visitInitializerList(InitializerList* node) override;
    void visitLabel(Label* node) override;
    void visitScopedAnnotatedSymbol(ScopedAnnotatedSymbol* node) override;
    void visitSelect(Select* node) override;
    void visitSubCall(SubCall* node) override;
    void visitSubReturn(SubReturn* node) override;
    void visitSymbol(Symbol* node) override;
    void visitUDTArrayDecl(UDTArrayDecl* node) override;
    void visitUDTDecl(UDTDecl* node) override;
    void visitUDTDeclBody(UDTDeclBody* node) override;
    void visitUDTFieldAssignment(UDTFieldAssignment* node) override;
    void visitUDTFieldOuter(UDTFieldOuter* node) override;
    void visitUDTFieldInner(UDTFieldInner* node) override;
    void visitUDTRef(UDTRef* node) override;
    void visitUDTVarDecl(UDTVarDecl* node) override;
    void visitUnaryOp(UnaryOp* node) override;
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

    virtual void visit(Node* node) = 0;
};

class GenericConstVisitor : public ConstVisitor
{
public:
    void visitAnnotatedSymbol(const AnnotatedSymbol* node) override;
    void visitArgList(const ArgList* node) override;
    void visitArrayAssignment(const ArrayAssignment* node) override;
    void visitArrayRef(const ArrayRef* node) override;
    void visitBinaryOp(const BinaryOp* node) override;
    void visitBlock(const Block* node) override;
    void visitCase(const Case* node) override;
    void visitCaseList(const CaseList* node) override;
    void visitCommandExpr(const CommandExpr* node) override;
    void visitCommandStmnt(const CommandStmnt* node) override;
    void visitConditional(const Conditional* node) override;
    void visitConstDecl(const ConstDecl* node) override;
    void visitConstDeclExpr(const ConstDeclExpr* node) override;
    void visitDefaultCase(const DefaultCase* node) override;
    void visitExit(const Exit* node) override;
    void visitForLoop(const ForLoop* node) override;
    void visitFuncCallExpr(const FuncCallExpr* node) override;
    void visitFuncCallExprOrArrayRef(const FuncCallExprOrArrayRef* node) override;
    void visitFuncCallStmnt(const FuncCallStmnt* node) override;
    void visitFuncDecl(const FuncDecl* node) override;
    void visitFuncExit(const FuncExit* node) override;
    void visitGoto(const Goto* node) override;
    void visitInfiniteLoop(const InfiniteLoop* node) override;
    void visitInitializerList(const InitializerList* node) override;
    void visitLabel(const Label* node) override;
    void visitScopedAnnotatedSymbol(const ScopedAnnotatedSymbol* node) override;
    void visitSelect(const Select* node) override;
    void visitSubCall(const SubCall* node) override;
    void visitSubReturn(const SubReturn* node) override;
    void visitSymbol(const Symbol* node) override;
    void visitUDTArrayDecl(const UDTArrayDecl* node) override;
    void visitUDTDecl(const UDTDecl* node) override;
    void visitUDTDeclBody(const UDTDeclBody* node) override;
    void visitUDTFieldOuter(const UDTFieldOuter* node) override;
    void visitUDTFieldInner(const UDTFieldInner* node) override;
    void visitUDTFieldAssignment(const UDTFieldAssignment* node) override;
    void visitUDTRef(const UDTRef* node) override;
    void visitUDTVarDecl(const UDTVarDecl* node) override;
    void visitUnaryOp(const UnaryOp* node) override;
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

    virtual void visit(const Node* node) = 0;
};

}
