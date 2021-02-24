#include "odb-compiler/tests/matchers/UDTRefEq.hpp"
#include "odb-compiler/ast/UDTRef.hpp"

UDTRefEqMatcher::UDTRefEqMatcher(const std::string& name)
    : expectedName_(name)
{}

bool UDTRefEqMatcher::MatchAndExplain(const odb::ast::UDTRef* node, testing::MatchResultListener* listener) const
{
    *listener
        << "node->name() == " << node->name();
    return node->name() == expectedName_;
}

void UDTRefEqMatcher::DescribeTo(::std::ostream* os) const
{
    *os << "node->name() equals " << expectedName_;
}

void UDTRefEqMatcher::DescribeNegationTo(::std::ostream* os) const
{
    *os << "node->name() does not equal " << expectedName_;
}
