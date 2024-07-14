#include "gmock/gmock.h"

extern "C" {
#include "odb-sdk/utf8.h"
}

struct Utf8EqMatcher : testing::MatcherInterface<const struct utf8>
{
    explicit Utf8EqMatcher(const char* expected) : expected(expected) {}

    bool
    MatchAndExplain(
        const struct utf8             str,
        testing::MatchResultListener* listener) const override;

    void
    DescribeTo(::std::ostream* os) const override;
    void
    DescribeNegationTo(::std::ostream* os) const override;

    std::string expected;
};

inline testing::Matcher<const struct utf8>
Utf8Eq(const char* expected)
{
    return testing::MakeMatcher(new Utf8EqMatcher(expected));
}
