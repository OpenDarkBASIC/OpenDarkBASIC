#pragma once

#include "odb-compiler/config.hpp"
#include "odb-compiler/ast/Type.hpp"
#include "odb-compiler/ast/Nodes.hpp"

namespace odb::ast {
namespace detail {
template <typename Callable, typename T, typename = void>
struct can_receive_t : std::false_type
{
};

template <typename Callable, typename T>
struct can_receive_t<Callable, T, std::void_t<decltype((std::declval<Callable>())(std::declval<T>()))>> : std::true_type
{
};
} // namespace detail

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

template <typename... Ts> struct FunctorVisitor : Visitor, Ts... {
    explicit FunctorVisitor(Ts... ts) : Ts(std::move(ts))... {}

    using Ts::operator()...;

#define X(nodeType) void visit##nodeType(nodeType* node) override {                     \
        if constexpr (detail::can_receive_t<FunctorVisitor<Ts...>, nodeType*>::value) { \
            (*this)(node);                                                              \
        }                                                                               \
    }
    ODB_AST_NODE_TYPE_LIST
#undef X
};
template <typename... Ts> FunctorVisitor(Ts...) -> FunctorVisitor<Ts...>;

template <typename... Ts> struct FunctorConstVisitor : ConstVisitor, Ts... {
    explicit FunctorConstVisitor(Ts... ts) : Ts(std::move(ts))... {}

    using Ts::operator()...;

#define X(nodeType) void visit##nodeType(const nodeType* node) override {                     \
        if constexpr (detail::can_receive_t<FunctorVisitor<Ts...>, const nodeType*>::value) { \
            (*this)(node);                                                                    \
        }                                                                                     \
    }
    ODB_AST_NODE_TYPE_LIST
#undef X
};
template <typename... Ts> FunctorConstVisitor(Ts...) -> FunctorConstVisitor<Ts...>;

enum class Traversal
{
    PreOrder,
    PostOrder
};

void visitAST(Node* node, Visitor& visitor, Traversal traversal = Traversal::PreOrder);
void visitAST(const Node* node, ConstVisitor& visitor, Traversal traversal = Traversal::PreOrder);

} // namespace odb::ast
