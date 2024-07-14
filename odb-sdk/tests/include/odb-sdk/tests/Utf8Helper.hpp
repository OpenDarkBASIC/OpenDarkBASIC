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

struct Utf8SpanEqMatcher : testing::MatcherInterface<const struct utf8_span&>
{
    explicit Utf8SpanEqMatcher(utf8_idx off, utf8_idx len)
        : expected({off, len})
    {
    }

    bool
    MatchAndExplain(
        const struct utf8_span&       span,
        testing::MatchResultListener* listener) const override
    {
        *listener << "{" << span.off << ", " << span.len << "}";
        return span.off == expected.off && span.len == expected.len;
    }

    void
    DescribeTo(::std::ostream* os) const override
    {
        *os << "Span equals {" << expected.off << ", " << expected.len << "}";
    }
    void
    DescribeNegationTo(::std::ostream* os) const override
    {
        *os << "Span does not equal {" << expected.off << ", " << expected.len
            << "}";
    }

    struct utf8_span expected;
};

inline testing::Matcher<const struct utf8_span&>
Utf8SpanEq(utf8_idx off, utf8_idx len)
{
    return testing::MakeMatcher(new Utf8SpanEqMatcher(off, len));
}

