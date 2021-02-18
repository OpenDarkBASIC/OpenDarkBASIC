#include "odb-compiler/astpost/ResolveLabels.hpp"
#include "odb-compiler/ast/Goto.hpp"
#include "odb-compiler/ast/Label.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Subroutine.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/Visitor.hpp"
#include "odb-sdk/Log.hpp"
#include <unordered_map>
#include <vector>
#include <string>

namespace odb::astpost {

// ----------------------------------------------------------------------------
namespace {
class Visitor : public ast::GenericVisitor
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

void Visitor::visitGotoSymbol(ast::GotoSymbol* node)
{
    gotos.push_back(node);
}

void Visitor::visitSubCallSymbol(ast::SubCallSymbol* node)
{
    subcalls.push_back(node);
}

void Visitor::visitLabel(ast::Label* node)
{
    const std::string& name = node->symbol()->name();
    const auto it = labels.insert({name, node});
    if (it.second == false)
    {
        Log::dbParserSyntaxError(
            node->location()->getFileLineColumn().c_str(), "Label ");
        Log::info.print(Log::FG_BRIGHT_WHITE, "`%s`", name.c_str());
        Log::info.print(" redefined\n");
        node->location()->printUnderlinedSection(Log::info);

        Log::dbParserLocationNote(
            it.first->second->location()->getFileLineColumn().c_str(),
            "Label previously defined here\n");
        it.first->second->location()->printUnderlinedSection(Log::info);

        errorOccurred = true;
    }
}
}

// ----------------------------------------------------------------------------
bool ResolveLabels::execute(ast::Node* node)
{
    Visitor gatherer;
    node->accept(&gatherer);
    if (gatherer.errorOccurred)
        return false;

    for (auto& gotoNode : gatherer.gotos)
    {
        const std::string& name = gotoNode->labelSymbol()->name();
        auto it = gatherer.labels.find(name);
        if (it == gatherer.labels.end())
        {
            Log::dbParserSyntaxError(
                gotoNode->location()->getFileLineColumn().c_str(),
                "Label ");
            Log::info.print(Log::FG_BRIGHT_WHITE, "`%s`", name.c_str());
            Log::info.print(" undefined\n");
            gotoNode->labelSymbol()->location()->printUnderlinedSection(Log::info);
            return false;
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
            Log::dbParserSyntaxError(
                subCallNode->location()->getFileLineColumn().c_str(),
                "Label ");
            Log::info.print(Log::FG_BRIGHT_WHITE, "`%s`", name.c_str());
            Log::info.print(" undefined\n");
            subCallNode->labelSymbol()->location()->printUnderlinedSection(Log::info);
            return false;
        }

        subCallNode->parent()->swapChild(subCallNode, new ast::SubCall(
            it->second, subCallNode->location()
        ));
    }

    return true;
}

}
