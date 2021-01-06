#include "odb-compiler/astpost/ResolveArrayFuncAmbiguity.hpp"
#include "odb-compiler/ast/ArrayDecl.hpp"
#include "odb-compiler/ast/ArrayRef.hpp"
#include "odb-compiler/ast/ExpressionList.hpp"
#include "odb-compiler/ast/FuncDecl.hpp"
#include "odb-compiler/ast/FuncCall.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/Visitor.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/parsers/db/ErrorPrinter.hpp"
#include "odb-sdk/Log.hpp"
#include <unordered_map>
#include <string>

namespace odb {
namespace astpost {

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
        log::dbParser(log::ERROR, "%s: Function `%s` redefined\n",
                      node->location()->getFileLineColumn().c_str(), name.c_str());
        db::printLocationHighlight(node->location());
        log::dbParser(log::NOTICE, "Function previously defined here\n");
        db::printLocationHighlight(ins.first->second->location());

        errorOccurred = true;
    }
    else
    {
        const auto it = arrayDecls.find(ins.first->first);
        if (it != arrayDecls.end())
        {
            log::dbParser(log::ERROR, "%s: Function `%s` has same name as array\n",
                          node->location()->getFileLineColumn().c_str(), name.c_str());
            db::printLocationHighlight(node->location());
            db::printLocationHighlight(it->second->location());

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
        log::dbParser(log::ERROR, "%s: Array `%s` redefined\n",
                      node->location()->getFileLineColumn().c_str(), name.c_str());
        db::printLocationHighlight(node->location());
        log::dbParser(log::NOTICE, "Array previously defined here\n");
        db::printLocationHighlight(ins.first->second->location());

        errorOccurred = true;
    }
    else
    {
        const auto it = funcDecls.find(ins.first->first);
        if (it != funcDecls.end())
        {
            log::dbParser(log::ERROR, "%s: Array `%s` has same name as function\n",
                          node->location()->getFileLineColumn().c_str(), name.c_str());
            db::printLocationHighlight(node->location());
            db::printLocationHighlight(it->second->location());

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

        log::dbParser(log::ERROR, "%s: Undefined array or function call `%s`\n",
                      amb->location()->getFileLineColumn().c_str(), name.c_str());
        db::printLocationHighlight(amb->location());
        return false;
    }

    return true;
}

}
}
