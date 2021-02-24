#pragma once

#include "gmock/gmock-matchers.h"

namespace odb::ast {
    class InitializerList;
}

class InitializerListCountEqMatcher : public testing::MatcherInterface<const odb::ast::InitializerList*>
{
public:
    explicit InitializerListCountEqMatcher(int expectedCount);
    bool MatchAndExplain(const odb::ast::InitializerList* node, testing::MatchResultListener* listener) const override;
    void DescribeTo(::std::ostream* os) const override;
    void DescribeNegationTo(::std::ostream* os) const override;

private:
    const int expectedCount_;
};

inline testing::Matcher<const odb::ast::InitializerList*> InitializerListCountEq(int expectedCount) {
    return testing::MakeMatcher(new InitializerListCountEqMatcher(expectedCount));
}
