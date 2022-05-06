#include "odb-compiler/ast/Visitor.hpp"
#include "odb-compiler/ast/ArgList.hpp"
#include "odb-compiler/ast/ArrayDecl.hpp"
#include "odb-compiler/ast/ArrayRef.hpp"
#include "odb-compiler/ast/Assignment.hpp"
#include "odb-compiler/ast/BinaryOp.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/CommandExpr.hpp"
#include "odb-compiler/ast/CommandStmnt.hpp"
#include "odb-compiler/ast/Conditional.hpp"
#include "odb-compiler/ast/ConstDecl.hpp"
#include "odb-compiler/ast/Exit.hpp"
#include "odb-compiler/ast/FuncCall.hpp"
#include "odb-compiler/ast/FuncDecl.hpp"
#include "odb-compiler/ast/Goto.hpp"
#include "odb-compiler/ast/Identifier.hpp"
#include "odb-compiler/ast/ImplicitCast.hpp"
#include "odb-compiler/ast/InitializerList.hpp"
#include "odb-compiler/ast/Label.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/Loop.hpp"
#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/ast/Program.hpp"
#include "odb-compiler/ast/ScopedIdentifier.hpp"
#include "odb-compiler/ast/SelectCase.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Subroutine.hpp"
#include "odb-compiler/ast/TreeIterator.hpp"
#include "odb-compiler/ast/UDTDecl.hpp"
#include "odb-compiler/ast/UDTField.hpp"
#include "odb-compiler/ast/UnaryOp.hpp"
#include "odb-compiler/ast/VarDecl.hpp"
#include "odb-compiler/ast/Variable.hpp"
#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/commands/Command.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
#define X(nodeType) void GenericVisitor::visit##nodeType(nodeType* node) { visit(node); }
ODB_AST_NODE_TYPE_LIST
#undef X

// ----------------------------------------------------------------------------
#define X(nodeType) void GenericConstVisitor::visit##nodeType(const nodeType* node) { visit(node); }
ODB_AST_NODE_TYPE_LIST
#undef X

// ----------------------------------------------------------------------------
void visitAST(Node* node, Visitor& visitor, Traversal traversal)
{
    switch (traversal)
    {
    case Traversal::PreOrder:
        for (Node* n : preOrderTraversal(node))
        {
            n->accept(&visitor);
        }
        break;
    case Traversal::PostOrder:
        for (Node* n : postOrderTraversal(node))
        {
            n->accept(&visitor);
        }
        break;
    default:
        break;
    }
}

// ----------------------------------------------------------------------------
void visitAST(const Node* node, ConstVisitor& visitor, Traversal traversal)
{
    switch (traversal)
    {
    case Traversal::PreOrder:
        for (const Node* n : preOrderTraversal(node))
        {
            n->accept(&visitor);
        }
        break;
    case Traversal::PostOrder:
        for (const Node* n : postOrderTraversal(node))
        {
            n->accept(&visitor);
        }
        break;
    default:
        break;
    }
}

}
