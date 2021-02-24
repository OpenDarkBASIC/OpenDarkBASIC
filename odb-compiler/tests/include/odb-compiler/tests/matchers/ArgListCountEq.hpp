#pragma once

#include "gmock/gmock-matchers.h"

namespace odb::ast {
    class ArgList;
}

class ArgListCountEqMatcher : public testing::MatcherInterface<const odb::ast::ArgList*>
{
public:
    explicit ArgListCountEqMatcher(int expectedCount);
    bool MatchAndExplain(const odb::ast::ArgList* node, testing::MatchResultListener* listener) const override;
    void DescribeTo(::std::ostream* os) const override;
    void DescribeNegationTo(::std::ostream* os) const override;

private:
    const int expectedCount_;
};

inline testing::Matcher<const odb::ast::ArgList*> ArgListCountEq(int expectedCount) {
    return testing::MakeMatcher(new ArgListCountEqMatcher(expectedCount));
}
