#include "odb-compiler/tests/matchers/CommandStmntEq.hpp"
#include "odb-compiler/ast/CommandStmnt.hpp"

CommandStmntEqMatcher::CommandStmntEqMatcher(std::string name)
    : expectedCommandName_(std::move(name))
{}

bool CommandStmntEqMatcher::MatchAndExplain(const odb::ast::CommandStmnt* node, testing::MatchResultListener* listener) const
{
    *listener << "node->commandName() == " << node->commandName();
    return node->commandName() == expectedCommandName_;
}

void CommandStmntEqMatcher::DescribeTo(::std::ostream* os) const
{
    *os << "node->command() equals " << expectedCommandName_;
}

void CommandStmntEqMatcher::DescribeNegationTo(::std::ostream* os) const
{
    *os << "node->command() does not equal " << expectedCommandName_;
}
