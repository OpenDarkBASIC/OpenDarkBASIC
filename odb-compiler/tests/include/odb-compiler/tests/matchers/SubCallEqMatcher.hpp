#pragma once

#include "gmock/gmock-matchers.h"

namespace odb::ast {
    class SubCall;
}

class SubCallEqMatcher : public testing::MatcherInterface<const odb::ast::SubCall*>
{
public:
    explicit SubCallEqMatcher(std::string labelName);
    bool MatchAndExplain(const odb::ast::SubCall* node, testing::MatchResultListener* listener) const override;
    void DescribeTo(::std::ostream* os) const override;
    void DescribeNegationTo(::std::ostream* os) const override;

private:
    const std::string expectedLabel_;
};

inline testing::Matcher<const odb::ast::SubCall*> SubCallEq(std::string labelName) {
    return testing::MakeMatcher(new SubCallEqMatcher(std::move(labelName)));
}
