#include "odb-compiler/astpost/EliminateBitwiseNotRHS.hpp"
#include "odb-compiler/ast/BinaryOp.hpp"
#include "odb-compiler/ast/ParentMap.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/TreeIterator.hpp"
#include "odb-compiler/ast/UnaryOp.hpp"
#include "odb-sdk/Log.hpp"

#define NO_SIDE_EFFECTS  \
    X(ArrayRef)          \
    X(BinaryOp)          \
    X(Conditional)       \
    X(ArgList)           \
    X(Identifier)        \
    X(ScopedIdentifier)  \
    X(UnaryOp)           \
    X(VarRef)

namespace odb::astpost {

namespace {
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
bool EliminateBitwiseNotRHS::execute(ast::Program* root)
{
    auto range = ast::preOrderTraversal(root);
    for (auto it = range.begin(); it != range.end(); ++it)
    {
        auto* op = dynamic_cast<ast::BinaryOp*>(static_cast<ast::Node*>(*it));
        if (!op || op->op() != ast::BinaryOpType::BITWISE_NOT)
        {
            continue;
        }

        SideEffectFinder finder;
        visitAST(op->rhs(), finder);
        if (finder.hasSideEffects)
        {
            Log::dbParserSemanticError(
                op->rhs()->location()->getFileLineColumn().c_str(),
                "RHS of bitwise-not operator causes side effects\n");
            op->rhs()->location()->printUnderlinedSection(Log::info);
            return false;
        }

        it.replaceNode(new ast::UnaryOp(
            ast::UnaryOpType::BITWISE_NOT, op->lhs(), op->location()
        ));
    }

    return true;
}

}
