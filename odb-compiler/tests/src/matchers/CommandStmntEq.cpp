#include "odb-compiler/tests/matchers/CommandStmntEq.hpp"
#include "odb-compiler/ast/CommandStmnt.hpp"

CommandStmntEqMatcher::CommandStmntEqMatcher(const std::string& name)
    : expectedCommand_(name)
{}

bool CommandStmntEqMatcher::MatchAndExplain(const odb::ast::CommandStmnt* node, testing::MatchResultListener* listener) const
{
    *listener << "node->command() == " << node->command();
    return node->command() == expectedCommand_;
}

void CommandStmntEqMatcher::DescribeTo(::std::ostream* os) const
{
    *os << "node->command() equals " << expectedCommand_;
}

void CommandStmntEqMatcher::DescribeNegationTo(::std::ostream* os) const
{
    *os << "node->command() does not equal " << expectedCommand_;
}
