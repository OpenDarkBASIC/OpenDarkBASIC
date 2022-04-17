#pragma once

#include "gmock/gmock-matchers.h"

namespace odb::ast {
    class Goto;
}

class GotoEqMatcher : public testing::MatcherInterface<const odb::ast::Goto*>
{
public:
    explicit GotoEqMatcher(std::string labelName);
    bool MatchAndExplain(const odb::ast::Goto* node, testing::MatchResultListener* listener) const override;
    void DescribeTo(::std::ostream* os) const override;
    void DescribeNegationTo(::std::ostream* os) const override;

private:
    const std::string expectedLabel_;
};

inline testing::Matcher<const odb::ast::Goto*> GotoEq(std::string labelName) {
    return testing::MakeMatcher(new GotoEqMatcher(std::move(labelName)));
}
