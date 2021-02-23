#include "odb-compiler/astpost/EliminateBitwiseNotRHS.hpp"
#include "odb-compiler/ast/BinaryOp.hpp"
#include "odb-compiler/ast/UnaryOp.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-sdk/Log.hpp"

#define NO_SIDE_EFFECTS       \
    X(AnnotatedSymbol)        \
    X(ArrayRef)               \
    X(BinaryOp)               \
    X(Conditional)            \
    X(ExpressionList)         \
    X(ScopedAnnotatedSymbol)  \
    X(Symbol)                 \
    X(UDTRef)                 \
    X(UnaryOp)                \
    X(VarRef)

namespace odb::astpost {

namespace {
class Visitor : public ast::GenericVisitor
{
public:
    void visitBinaryOp(ast::BinaryOp* node) override final {
        if (node->op() == ast::BinaryOp::BITWISE_NOT)
            ops.push_back(node);
    }

    void visit(ast::Node* node) override final { /* don't care */ }

public:
    std::vector<Reference<ast::BinaryOp>> ops;
};

class SideEffectFinder : public ast::GenericConstVisitor
{
public:
#define X(name) void visit##name(const ast::name* node) override final {}
    NO_SIDE_EFFECTS
#undef X
#define X(dbname, cppname) void visit##dbname##Literal(const ast::dbname##Literal* node) override final {}
    ODB_DATATYPE_LIST
#undef X

    void visit(const ast::Node* node) override final { hasSideEffects = true; }

public:
    bool hasSideEffects = false;
};
}

// ----------------------------------------------------------------------------
bool EliminateBitwiseNotRHS::execute(ast::Node* node)
{
    Visitor gatherer;
    node->accept(&gatherer);

    for (auto& op : gatherer.ops)
    {
        SideEffectFinder finder;
        op->rhs()->accept(&finder);
        if (finder.hasSideEffects)
        {
            Log::dbParserSemanticError(
                op->rhs()->location()->getFileLineColumn().c_str(),
                "RHS of bitwise-not operator causes side effects\n");
            op->rhs()->location()->printUnderlinedSection(Log::info);
            return false;
        }

        op->parent()->swapChild(op, new ast::UnaryOp(
            ast::UnaryOp::BITWISE_NOT, op->lhs(), op->location()
        ));
    }

    return true;
}

}
