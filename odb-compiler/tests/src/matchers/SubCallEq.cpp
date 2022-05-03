#include <utility>

#include "odb-compiler/tests/matchers/SubCallEqMatcher.hpp"
#include "odb-compiler/ast/Subroutine.hpp"
#include "odb-compiler/ast/Label.hpp"
#include "odb-compiler/ast/Identifier.hpp"

SubCallEqMatcher::SubCallEqMatcher(std::string labelName)
    : expectedLabel_(std::move(labelName))
{}

bool SubCallEqMatcher::MatchAndExplain(const odb::ast::SubCall* node, testing::MatchResultListener* listener) const
{
    *listener << "node->label()->name() == " << node->label()->identifier()->name();
    return node->label()->identifier()->name() == expectedLabel_;
}

void SubCallEqMatcher::DescribeTo(::std::ostream* os) const
{
    *os << "node->label()->identifier()->name() equals " << expectedLabel_;
}

void SubCallEqMatcher::DescribeNegationTo(::std::ostream* os) const
{
    *os << "node->label()->identifier()->name() does not equal " << expectedLabel_;
}
