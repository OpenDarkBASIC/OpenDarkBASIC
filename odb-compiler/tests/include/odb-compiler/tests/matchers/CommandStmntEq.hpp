#pragma once

#include "gmock/gmock-matchers.h"

namespace odb::ast {
    class CommandStmnt;
}

class CommandStmntEqMatcher : public testing::MatcherInterface<const odb::ast::CommandStmnt*>
{
public:
    explicit CommandStmntEqMatcher(const std::string& name);
    bool MatchAndExplain(const odb::ast::CommandStmnt* node, testing::MatchResultListener* listener) const override;
    void DescribeTo(::std::ostream* os) const override;
    void DescribeNegationTo(::std::ostream* os) const override;

private:
    const std::string expectedCommand_;
};

inline testing::Matcher<const odb::ast::CommandStmnt*> CommandStmntEq(const std::string& name) {
    return testing::MakeMatcher(new CommandStmntEqMatcher(name));
}
