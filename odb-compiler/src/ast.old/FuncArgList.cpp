#include "odb-compiler/ast/FuncArgList.hpp"
#include "odb-compiler/ast/VarDecl.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Visitor.hpp"

namespace odb::ast {

// ----------------------------------------------------------------------------
FuncArgList::FuncArgList(Program* program, SourceLocation* location) :
    Node(program, location)
{
}

// ----------------------------------------------------------------------------
FuncArgList::FuncArgList(Program* program, SourceLocation* location, VarDecl* initialVar) :
    Node(program, location)
{
    appendVarDecl(initialVar);
}

// ----------------------------------------------------------------------------
void FuncArgList::appendVarDecl(VarDecl* expr)
{
    varDecls_.emplace_back(expr);

    location()->unionize(expr->location());
}

// ----------------------------------------------------------------------------
const std::vector<Reference<VarDecl>>& FuncArgList::varDecls() const
{
    return varDecls_;
}

// ----------------------------------------------------------------------------
std::string FuncArgList::toString() const
{
    return "FuncArgList(" + std::to_string(varDecls_.size()) + ")";
}

// ----------------------------------------------------------------------------
void FuncArgList::accept(Visitor* visitor)
{
    visitor->visitFuncArgList(this);
}
void FuncArgList::accept(ConstVisitor* visitor) const
{
    visitor->visitFuncArgList(this);
}

// ----------------------------------------------------------------------------
Node::ChildRange FuncArgList::children()
{
    ChildRange children;
    for (VarDecl* e : varDecls_)
    {
        children.push_back(e);
    }
    return children;
}

// ----------------------------------------------------------------------------
void FuncArgList::swapChild(const Node* oldNode, Node* newNode)
{
    for (auto& expr : varDecls_)
        if (expr == oldNode)
        {
            expr = dynamic_cast<VarDecl*>(newNode);
            return;
        }

    assert(false);
}

// ----------------------------------------------------------------------------
Node* FuncArgList::duplicateImpl() const
{
    FuncArgList* el = new FuncArgList(program(), location());
    for (const auto& expr : varDecls_)
        el->appendVarDecl(expr->duplicate<VarDecl>());
    return el;
}

}
