#include "odb-compiler/astpost/ResolveLabels.hpp"
#include "odb-compiler/ast/Goto.hpp"
#include "odb-compiler/ast/Label.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Subroutine.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/Visitor.hpp"
#include "odb-compiler/parsers/db/ErrorPrinter.hpp"
#include "odb-sdk/Log.hpp"
#include <unordered_map>
#include <vector>
#include <string>

namespace odb::astpost {

// ----------------------------------------------------------------------------
class Gatherer : public ast::GenericVisitor
{
public:
    void visitGotoSymbol(ast::GotoSymbol* node) override;
    void visitSubCallSymbol(ast::SubCallSymbol* node) override;
    /*void visitRestoreSymbol(const ast::RestoreSymbol* node) override;*/
    void visitLabel(ast::Label* node) override;

    void visit(ast::Node* node) override final { /* don't care */ }

public:
    std::vector<const ast::GotoSymbol*> gotos;
    std::vector<const ast::SubCallSymbol*> subcalls;
    std::unordered_map<std::string, ast::Label*> labels;

    bool errorOccurred = false;
};

void Gatherer::visitGotoSymbol(ast::GotoSymbol* node)
{
    gotos.push_back(node);
}

void Gatherer::visitSubCallSymbol(ast::SubCallSymbol* node)
{
    subcalls.push_back(node);
}

void Gatherer::visitLabel(ast::Label* node)
{
    const std::string& name = node->symbol()->name();
    const auto it = labels.insert({name, node});
    if (it.second == false)
    {
        log::dbParser(log::ERROR, "%s: Label `%s` redefined\n",
                      node->location()->getFileLineColumn().c_str(), name.c_str());
        log::dbParser(log::NOTICE, "Label previously defined here\n");
        db::printLocationHighlight(node->location());

        errorOccurred = true;
    }
}

// ----------------------------------------------------------------------------
bool ResolveLabels::execute(ast::Node* node)
{
    Gatherer gatherer;
    node->accept(&gatherer);
    if (gatherer.errorOccurred)
        return false;

    for (auto& gotoNode : gatherer.gotos)
    {
        const std::string& name = gotoNode->labelSymbol()->name();
        auto it = gatherer.labels.find(name);
        if (it == gatherer.labels.end())
        {
            log::dbParser(log::ERROR, "%s: Label `%s` undefined\n",
                          gotoNode->location()->getFileLineColumn().c_str(), name.c_str());
            db::printLocationHighlight(gotoNode->labelSymbol()->location());
            continue;
        }

        gotoNode->parent()->swapChild(gotoNode, new ast::Goto(
            it->second, gotoNode->location()
        ));
    }

    for (auto& subCallNode : gatherer.subcalls)
    {
        const std::string& name = subCallNode->labelSymbol()->name();
        auto it = gatherer.labels.find(name);
        if (it == gatherer.labels.end())
        {
            log::dbParser(log::ERROR, "%s: Label `%s` undefined\n",
                          subCallNode->location()->getFileLineColumn().c_str(), name.c_str());
            db::printLocationHighlight(subCallNode->labelSymbol()->location());
            continue;
        }

        subCallNode->parent()->swapChild(subCallNode, new ast::SubCall(
            it->second, subCallNode->location()
        ));
    }

    return true;
}

}
