#include "odb-compiler/ast/ArrayRef.hpp"
#include "odb-compiler/ast/Command.hpp"
#include "odb-compiler/ast/FuncCall.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/ArgList.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/UDTField.hpp"
#include "odb-compiler/ast/VarRef.hpp"
#include "odb-compiler/astpost/ValidateUDTFieldNames.hpp"
#include "odb-compiler/commands/Command.hpp"
#include "odb-sdk/Log.hpp"

namespace odb::astpost {

// ----------------------------------------------------------------------------
namespace {
class Visitor : public ast::GenericConstVisitor
{
public:
    void visitUDTFieldOuter(const ast::UDTFieldOuter* node) override final { checkExpr(node->left()); }
    void visitUDTFieldInner(const ast::UDTFieldInner* node) override final { checkExpr(node->left()); }
    void visit(const ast::Node* node) override final { /* don't care */ }

    bool check(const ast::VarRef* varRef);
    bool check(const ast::ArrayRef* arrayRef);
    bool check(const ast::FuncCallExpr* func);
    bool check(const ast::FuncCallStmnt* func);
    bool check(const ast::FuncCallExprOrArrayRef* func);
    bool check(const ast::CommandExpr* cmd);
    bool check(const ast::CommandStmnt* cmd);

    void checkExpr(const ast::Expression* node);
    void checkAnnotation(const ast::AnnotatedSymbol* sym);

    bool success = true;
};

// ----------------------------------------------------------------------------
bool Visitor::check(const ast::VarRef* varRef)
{
    if (varRef)
        return checkAnnotation(varRef->symbol()), true;
    return false;
}
bool Visitor::check(const ast::ArrayRef* arrRef)
{
    if (arrRef)
        return checkAnnotation(arrRef->symbol()), true;
    return false;
}
bool Visitor::check(const ast::FuncCallExpr* func)
{
    if (func)
        return checkAnnotation(func->symbol()), true;
    return false;
}
bool Visitor::check(const ast::FuncCallStmnt* func)
{
    if (func)
        return checkAnnotation(func->symbol()), true;
    return false;
}
bool Visitor::check(const ast::FuncCallExprOrArrayRef* func)
{
    if (func)
        return checkAnnotation(func->symbol()), true;
    return false;
}
bool Visitor::check(const ast::CommandExpr* cmd)
{
    if (cmd == nullptr)
        return false;

    char c = cmd->command().back();
    if (ast::isTypeAnnotation(c))
    {
        Log::dbParserSemanticError(
            cmd->location()->getFileLineColumn().c_str(),
            "Misleading command annotation: Command returns a UDT, but UDTs cannot be annotated\n");
        cmd->location()->printUnderlinedSection(Log::info);
        success = false;
    }

    return true;
}
bool Visitor::check(const ast::CommandStmnt* cmd)
{
    if (cmd == nullptr)
        return false;

    char c = cmd->command().back();
    if (ast::isTypeAnnotation(c))
    {
        Log::dbParserSemanticError(
            cmd->location()->getFileLineColumn().c_str(),
            "Misleading command annotation: Command returns a UDT, but UDTs cannot be annotated\n");
        cmd->location()->printUnderlinedSection(Log::info);
        success = false;
    }

    return true;
}

// ----------------------------------------------------------------------------
void Visitor::checkExpr(const ast::Expression* node)
{
    if (check(dynamic_cast<const ast::VarRef*>(node))) return;
    if (check(dynamic_cast<const ast::ArrayRef*>(node))) return;
    if (check(dynamic_cast<const ast::FuncCallExpr*>(node))) return;
    if (check(dynamic_cast<const ast::FuncCallStmnt*>(node))) return;
    if (check(dynamic_cast<const ast::FuncCallExprOrArrayRef*>(node))) return;
    if (check(dynamic_cast<const ast::CommandExpr*>(node))) return;
    if (check(dynamic_cast<const ast::CommandStmnt*>(node))) return;
}

// ----------------------------------------------------------------------------
void Visitor::checkAnnotation(const ast::AnnotatedSymbol* sym)
{
    if (sym->annotation() != ast::Symbol::Annotation::NONE)
    {
        Log::dbParserSyntaxError(sym->location()->getFileLineColumn().c_str(),
            "UDTs cannot be annotated\n");
        sym->location()->printUnderlinedSection(Log::info);
        success = false;
    }
}

}

// ----------------------------------------------------------------------------
bool ValidateUDTFieldNames::execute(ast::Node* node)
{
    Visitor visitor;
    node->accept(&visitor);
    return visitor.success;
}

}
