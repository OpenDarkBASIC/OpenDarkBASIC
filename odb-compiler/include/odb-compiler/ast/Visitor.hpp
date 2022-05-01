#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Type.hpp"
#include "odb-compiler/ast/Nodes.hpp"

namespace odb::ast {

class Node;

#define X(nodeType) class nodeType;
ODB_AST_NODE_TYPE_LIST
#undef X

class ODBCOMPILER_PUBLIC_API Visitor
{
public:
    virtual ~Visitor() = default;

#define X(nodeType) virtual void visit##nodeType(nodeType* node) = 0;
    ODB_AST_NODE_TYPE_LIST
#undef X
};

class ODBCOMPILER_PUBLIC_API ConstVisitor
{
public:
    virtual ~ConstVisitor() = default;

#define X(nodeType) virtual void visit##nodeType(const nodeType* node) = 0;
    ODB_AST_NODE_TYPE_LIST
#undef X
};

class ODBCOMPILER_PUBLIC_API GenericVisitor : public Visitor
{
public:
#define X(nodeType) void visit##nodeType(nodeType* node) override;
    ODB_AST_NODE_TYPE_LIST
#undef X

    virtual void visit(Node* node) = 0;
};

class ODBCOMPILER_PUBLIC_API GenericConstVisitor : public ConstVisitor
{
public:
#define X(nodeType) void visit##nodeType(const nodeType* node) override;
    ODB_AST_NODE_TYPE_LIST
#undef X

    virtual void visit(const Node* node) = 0;
};

enum class Traversal
{
    PreOrder,
    PostOrder
};

void visitAST(Node* node, Visitor& visitor, Traversal traversal = Traversal::PreOrder);
void visitAST(const Node* node, ConstVisitor& visitor, Traversal traversal = Traversal::PreOrder);

} // namespace odb::ast
