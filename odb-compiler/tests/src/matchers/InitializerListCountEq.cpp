#include "odb-compiler/tests/matchers/InitializerListCountEq.hpp"
#include "odb-compiler/ast/InitializerList.hpp"

InitializerListCountEqMatcher::InitializerListCountEqMatcher(int expectedCount)
    : expectedCount_(expectedCount)
{}

bool InitializerListCountEqMatcher::MatchAndExplain(const odb::ast::InitializerList* node, testing::MatchResultListener* listener) const
{
    *listener << "node->expressions().size() equals " << node->expressions().size();
    return node->expressions().size() == expectedCount_;
}

void InitializerListCountEqMatcher::DescribeTo(::std::ostream* os) const
{
    *os << "node->expressions().size() equals " << expectedCount_;
}

void InitializerListCountEqMatcher::DescribeNegationTo(::std::ostream* os) const
{
    *os << "node->expressions().size() does not equal " << expectedCount_;
}
