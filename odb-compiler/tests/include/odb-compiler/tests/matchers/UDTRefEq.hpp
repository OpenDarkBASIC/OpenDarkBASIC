#pragma once

#include "gmock/gmock-matchers.h"

namespace odb::ast {
    class UDTRef;
}

class UDTRefEqMatcher : public testing::MatcherInterface<const odb::ast::UDTRef*>
{
public:
    explicit UDTRefEqMatcher(const std::string& name);
    bool MatchAndExplain(const odb::ast::UDTRef* node, testing::MatchResultListener* listener) const override;
    void DescribeTo(::std::ostream* os) const override;
    void DescribeNegationTo(::std::ostream* os) const override;

private:
    const std::string expectedName_;
};

inline testing::Matcher<const odb::ast::UDTRef*> UDTRefEq(const std::string& name) {
    return testing::MakeMatcher(new UDTRefEqMatcher(name));
}
