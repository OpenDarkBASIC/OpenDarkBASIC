#include <utility>

#include "odb-compiler/tests/matchers/GotoEqMatcher.hpp"
#include "odb-compiler/ast/Goto.hpp"
#include "odb-compiler/ast/Identifier.hpp"

GotoEqMatcher::GotoEqMatcher(std::string labelName)
    : expectedLabel_(std::move(labelName))
{}

bool GotoEqMatcher::MatchAndExplain(const odb::ast::Goto* node, testing::MatchResultListener* listener) const
{
    *listener << "node->label()->name() == " << node->label()->name();
    return node->label()->name() == expectedLabel_;
}

void GotoEqMatcher::DescribeTo(::std::ostream* os) const
{
    *os << "node->label()->identifier()->name() equals " << expectedLabel_;
}

void GotoEqMatcher::DescribeNegationTo(::std::ostream* os) const
{
    *os << "node->label()->identifier()->name() does not equal " << expectedLabel_;
}
