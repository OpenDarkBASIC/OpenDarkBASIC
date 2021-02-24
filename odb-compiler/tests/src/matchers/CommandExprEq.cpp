#include "odb-compiler/tests/matchers/CommandExprEq.hpp"
#include "odb-compiler/ast/CommandExpr.hpp"

CommandExprEqMatcher::CommandExprEqMatcher(const std::string& name)
    : expectedCommand_(name)
{}

bool CommandExprEqMatcher::MatchAndExplain(const odb::ast::CommandExpr* node, testing::MatchResultListener* listener) const
{
    *listener << "node->command() == " << node->command();
    return node->command() == expectedCommand_;
}

void CommandExprEqMatcher::DescribeTo(::std::ostream* os) const
{
    *os << "node->command() equals " << expectedCommand_;
}

void CommandExprEqMatcher::DescribeNegationTo(::std::ostream* os) const
{
    *os << "node->command() does not equal " << expectedCommand_;
}
