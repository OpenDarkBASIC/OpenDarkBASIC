#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Type.hpp"

/*!
 * @brief All of the DarkBASIC AST nodes
 */
#define ODB_LITERAL_AST_NODE(dbname, cppname) X(dbname##Literal)
#define ODB_AST_NODE_TYPE_LIST_IMPL(X)  \
    X(ArgList)                          \
    X(ArrayAssignment)                  \
    X(ArrayDecl)                        \
    X(ArrayRef)                         \
    X(BinaryOp)                         \
    X(Block)                            \
    X(Case)                             \
    X(CaseList)                         \
    X(CommandExpr)                      \
    X(CommandStmnt)                     \
    X(Conditional)                      \
    X(ConstDecl)                        \
    X(ConstDeclExpr)                    \
    X(DefaultCase)                      \
    X(Exit)                             \
    X(ForLoop)                          \
    X(FuncCallExpr)                     \
    X(FuncCallExprOrArrayRef)           \
    X(FuncCallStmnt)                    \
    X(FuncDecl)                         \
    X(FuncExit)                         \
    X(Goto)                             \
    X(Identifier)                       \
    X(InfiniteLoop)                     \
    X(InitializerList)                  \
    X(Label)                            \
    X(ScopedIdentifier)                 \
    X(Select)                           \
    X(SubCall)                          \
    X(SubReturn)                        \
    X(VarDecl)                          \
    X(UDTDecl)                          \
    X(UDTDeclBody)                      \
    X(UDTFieldAssignment)               \
    X(UDTFieldOuter)                    \
    X(UDTFieldInner)                    \
    X(UnaryOp)                          \
    X(UntilLoop)                        \
    X(VarAssignment)                    \
    X(VarRef)                           \
    X(WhileLoop)                        \
    ODB_DATATYPE_LIST_IMPL(ODB_LITERAL_AST_NODE)

#define ODB_AST_NODE_TYPE_LIST \
    ODB_AST_NODE_TYPE_LIST_IMPL(X)