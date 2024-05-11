#include "odb-compiler/ast/Expression.hpp"
#include "odb-compiler/ast/SelectCase.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/astpost/EnforceSingleDefaultCase.hpp"
#include "odb-sdk/Log.hpp"

namespace odb::astpost {

// ----------------------------------------------------------------------------
namespace {
class Visitor : public ast::GenericConstVisitor
{
public:
    void visitSelect(const ast::Select* select) override final;
    void visit(const ast::Node* node) override final { /* don't care */ }

    bool success = true;
};

// ----------------------------------------------------------------------------
void Visitor::visitSelect(const ast::Select* select)
{
    ast::CaseList* list = select->cases();
    if (list->defaultCases().size() == 1)
        return;

    Reference<ast::SourceLocation> loc = select->beginSelectLocation()->duplicate();
    loc->unionize(select->expression()->location());

    Log::dbParserSyntaxError(
        loc->getFileLineColumn().c_str(),
        "select statement has multiple default cases\n");
    loc->printUnderlinedSection(Log::info);

    size_t i = 0;
    for (const auto& case_ : list->defaultCases())
    {
        static const char* table[] = {"first", "second", "third"};
        Log::dbParserLocationNote(
            case_->beginCaseLocation()->getFileLineColumn().c_str(),
            "%s default case defined here\n", table[i++]);
        case_->beginCaseLocation()->printUnderlinedSection(Log::info);
        if (i > 2)
            break;
    }

    if (i < list->defaultCases().size())
        Log::dbParserNotice("%d more default case(s) were omitted\n", list->defaultCases().size() - i);

    success = false;
}
}

// ----------------------------------------------------------------------------
bool EnforceSingleDefaultCase::execute(ast::Program* root)
{
    Visitor visitor;
    visitAST(root, visitor);
    return visitor.success;
}

}
