#include "odb-compiler/tests/matchers/ArgListCountEq.hpp"
#include "odb-compiler/ast/ArgList.hpp"

ArgListCountEqMatcher::ArgListCountEqMatcher(int expectedCount)
    : expectedCount_(expectedCount)
{}

bool ArgListCountEqMatcher::MatchAndExplain(const odb::ast::ArgList* node, testing::MatchResultListener* listener) const
{
    *listener << "node->expressions().size() equals " << node->expressions().size();
    return node->expressions().size() == expectedCount_;
}

void ArgListCountEqMatcher::DescribeTo(::std::ostream* os) const
{
    *os << "node->expressions().size() equals " << expectedCount_;
}

void ArgListCountEqMatcher::DescribeNegationTo(::std::ostream* os) const
{
    *os << "node->expressions().size() does not equal " << expectedCount_;
}
