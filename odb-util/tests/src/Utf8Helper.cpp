#include "odb-util/tests/Utf8Helper.hpp"

bool
Utf8EqMatcher::MatchAndExplain(
    const struct utf8 str, testing::MatchResultListener* listener) const
{
    *listener << "Utf8: \"" << std::string(str.data, str.len) << "\"";
    return std::string(str.data, str.len) == expected;
}

void
Utf8EqMatcher::DescribeTo(::std::ostream* os) const
{
    *os << "Utf8 equals: \"" << expected << "\"";
}

void
Utf8EqMatcher::DescribeNegationTo(::std::ostream* os) const
{
    *os << "Utf8 does not equal: \"" << expected << "\"";
}
