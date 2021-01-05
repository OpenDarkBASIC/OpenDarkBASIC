#include "odb-compiler/astpost/EliminateBitwiseNotRHS.hpp"
#include "odb-compiler/parsers/db/ErrorPrinter.hpp"
#include "odb-compiler/ast/BinaryOp.hpp"
#include "odb-compiler/ast/UnaryOp.hpp"

#define NO_SIDE_EFFECTS       \
    X(AnnotatedSymbol)        \
    X(ArrayRef)               \
    X(Conditional)            \
    X(ExpressionList)         \
    X(ScopedSymbol)           \
    X(ScopedAnnotatedSymbol)  \
    X(Symbol)                 \
    X(UDTRef)                 \
    X(VarRef)

namespace odb::astpost {

class Gatherer : public ast::GenericVisitor
{
public:
    void visitBinaryOpBitwiseNot(ast::BinaryOpBitwiseNot* node) override final {
        ops.push_back(node);
    }

    void visit(ast::Node* node) override final { /* don't care */ }

public:
    std::vector<Reference<ast::BinaryOpBitwiseNot>> ops;
};

class SideEffectAnalyzer : public ast::GenericVisitor
{
public:
#define X(name) void visit##name(ast::name* node) override final {}
    NO_SIDE_EFFECTS
#undef X

    void visit(ast::Node* node) override final { hasSideEffects = true; }

public:
    bool hasSideEffects = false;
};

// ----------------------------------------------------------------------------
bool EliminateBitwiseNotRHS::execute(ast::Node* node)
{
    Gatherer gatherer;
    node->accept(&gatherer);

    for (auto& op : gatherer.ops)
    {
        SideEffectAnalyzer a;
        op->rhs()->accept(&a);
        if (a.hasSideEffects)
        {
            log::dbParser(log::ERROR, "Side effects of right operand of binary bitwise-not operator would be eliminated\n");
            db::printLocationHighlight(op->rhs()->location());
            return false;
        }

        op->parent()->swapChild(op, new ast::UnaryOpBitwiseNot(
            op->lhs(), op->location()
        ));
    }

    return true;
}

}
