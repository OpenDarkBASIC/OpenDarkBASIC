#include "odb-compiler/tests/matchers/CommandExprEq.hpp"
#include "odb-compiler/ast/CommandExpr.hpp"

CommandExprEqMatcher::CommandExprEqMatcher(std::string name)
    : expectedCommandName_(std::move(name))
{}

bool CommandExprEqMatcher::MatchAndExplain(const odb::ast::CommandExpr* node, testing::MatchResultListener* listener) const
{
    *listener << "node->commandName() == " << node->commandName();
    return node->commandName() == expectedCommandName_;
}

void CommandExprEqMatcher::DescribeTo(::std::ostream* os) const
{
    *os << "node->command() equals " << expectedCommandName_;
}

void CommandExprEqMatcher::DescribeNegationTo(::std::ostream* os) const
{
    *os << "node->command() does not equal " << expectedCommandName_;
}
