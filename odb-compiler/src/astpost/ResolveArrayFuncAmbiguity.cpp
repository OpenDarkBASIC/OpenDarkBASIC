#include "odb-compiler/astpost/ResolveArrayFuncAmbiguity.hpp"
#include "odb-compiler/ast/ArrayDecl.hpp"
#include "odb-compiler/ast/ArrayRef.hpp"
#include "odb-compiler/ast/ExpressionList.hpp"
#include "odb-compiler/ast/FuncDecl.hpp"
#include "odb-compiler/ast/FuncCall.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/Visitor.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-sdk/Log.hpp"
#include <unordered_map>
#include <string>

namespace odb::astpost {

// ----------------------------------------------------------------------------
namespace {
class Gatherer : public ast::GenericVisitor
{
public:
    void visitFuncDecl(ast::FuncDecl* node) override final;
    void visitArrayDecl(ast::ArrayDecl* node);
    void visitFuncCallExprOrArrayRef(ast::FuncCallExprOrArrayRef* node) override final;

#define X(dbname, cppname) void visit##dbname##ArrayDecl(ast::dbname##ArrayDecl* node) override final;
    ODB_DATATYPE_LIST
#undef X

    void visit(ast::Node* node) override final { /* don't care */ }

public:
    std::unordered_map<std::string, Reference<ast::ArrayDecl>> arrayDecls;
    std::unordered_map<std::string, Reference<ast::FuncDecl>> funcDecls;
    std::vector<Reference<ast::FuncCallExprOrArrayRef>> funcCallOrArrayRefs;

    bool errorOccurred = false;
};

void Gatherer::visitFuncDecl(ast::FuncDecl* node)
{
    const std::string& name = node->symbol()->name();
    const auto ins = funcDecls.insert({name, node});
    if (ins.second == false)
    {
        Log::dbParserSyntaxError(
            node->location()->getFileLineColumn().c_str(),
            "Function ");
        Log::info.print(Log::FG_BRIGHT_WHITE, "`%s`", name.c_str());
        Log::info.print(" redefined\n");
        node->location()->printUnderlinedSection(Log::info);

        Log::dbParserLocationNote(
            ins.first->second->location()->getFileLineColumn().c_str(),
            "Function previously defined here\n");
        ins.first->second->location()->printUnderlinedSection(Log::info);

        errorOccurred = true;
    }
    else
    {
        const auto it = arrayDecls.find(ins.first->first);
        if (it != arrayDecls.end())
        {
            Log::dbParserSyntaxError(
                node->location()->getFileLineColumn().c_str(),
                "Function ");
            Log::info.print(Log::FG_BRIGHT_WHITE, "`%s`", name.c_str());
            Log::info.print(" has same name as array\n");
            node->location()->printUnderlinedSection(Log::info);
            it->second->location()->printUnderlinedSection(Log::info);

            errorOccurred = true;
        }
    }
}

void Gatherer::visitArrayDecl(ast::ArrayDecl* node)
{
    const std::string& name = node->symbol()->name();
    const auto ins = arrayDecls.insert({name, node});
    if (ins.second == false)
    {
        Log::dbParserSyntaxError(
            node->location()->getFileLineColumn().c_str(),
            "Array ");
        Log::info.print(Log::FG_BRIGHT_WHITE, "`%s`", name.c_str());
        Log::info.print(" redefined\n");
        node->location()->printUnderlinedSection(Log::info);

        Log::dbParserLocationNote(
            ins.first->second->location()->getFileLineColumn().c_str(),
            "Array previously defined here\n");
        ins.first->second->location()->printUnderlinedSection(Log::info);

        errorOccurred = true;
    }
    else
    {
        const auto it = funcDecls.find(ins.first->first);
        if (it != funcDecls.end())
        {
            Log::dbParserSyntaxError(
                node->location()->getFileLineColumn().c_str(),
                "Array ");
            Log::info.print(Log::FG_BRIGHT_WHITE, "`%s`", name.c_str());
            Log::info.print(" has same name as function\n");
            node->location()->printUnderlinedSection(Log::info);
            it->second->location()->printUnderlinedSection(Log::info);

            errorOccurred = true;
        }
    }
}

void Gatherer::visitFuncCallExprOrArrayRef(ast::FuncCallExprOrArrayRef* node)
{
    funcCallOrArrayRefs.push_back(node);
}

#define X(dbname, cppname)                                                    \
    void Gatherer::visit##dbname##ArrayDecl(ast::dbname##ArrayDecl* node) {   \
        visitArrayDecl(node);                                                 \
    }
ODB_DATATYPE_LIST
#undef X
}

// ----------------------------------------------------------------------------
bool ResolveArrayFuncAmbiguity::execute(ast::Node* node)
{
    Gatherer gatherer;
    node->accept(&gatherer);
    if (gatherer.errorOccurred)
        return false;

    for (auto& amb : gatherer.funcCallOrArrayRefs)
    {
        const std::string& name = amb->symbol()->name();
        const auto array = gatherer.arrayDecls.find(name);
        if (array != gatherer.arrayDecls.end())
        {
            assert(amb->args().notNull());
            amb->parent()->swapChild(amb, new ast::ArrayRef(
                amb->symbol(), amb->args(), amb->location()
            ));
            continue;
        }

        const auto func = gatherer.funcDecls.find(name);
        if (func != gatherer.funcDecls.end())
        {
            amb->parent()->swapChild(amb, new ast::FuncCallExpr(
                amb->symbol(), amb->args(), amb->location()
            ));
            continue;
        }

        Log::dbParserSyntaxError(
            amb->location()->getFileLineColumn().c_str(),
            "Undefined array or function call ");
        Log::info.print(Log::FG_BRIGHT_WHITE, "`%s`", name.c_str());
        Log::info.print("\n");
        amb->location()->printUnderlinedSection(Log::info);

        return false;
    }

    return true;
}

}
