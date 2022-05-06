#pragma once

#include "gmock/gmock-matchers.h"

namespace odb::ast {
    class CommandExpr;
}

class CommandExprEqMatcher : public testing::MatcherInterface<const odb::ast::CommandExpr*>
{
public:
    explicit CommandExprEqMatcher(std::string name);
    bool MatchAndExplain(const odb::ast::CommandExpr* node, testing::MatchResultListener* listener) const override;
    void DescribeTo(::std::ostream* os) const override;
    void DescribeNegationTo(::std::ostream* os) const override;

private:
    const std::string expectedCommandName_;
};

inline testing::Matcher<const odb::ast::CommandExpr*> CommandExprEq(std::string name) {
    return MakeMatcher(new CommandExprEqMatcher(std::move(name)));
}
