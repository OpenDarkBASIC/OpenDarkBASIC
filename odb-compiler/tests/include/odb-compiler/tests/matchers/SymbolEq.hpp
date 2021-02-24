#pragma once

#include "gmock/gmock-matchers.h"

namespace odb::ast {
    class Symbol;
}

class SymbolEqMatcher : public testing::MatcherInterface<const odb::ast::Symbol*>
{
public:
    explicit SymbolEqMatcher(const std::string& name);
    bool MatchAndExplain(const odb::ast::Symbol* node, testing::MatchResultListener* listener) const override;
    void DescribeTo(::std::ostream* os) const override;
    void DescribeNegationTo(::std::ostream* os) const override;

private:
    const std::string expectedName_;
};

inline testing::Matcher<const odb::ast::Symbol*> SymbolEq(const std::string& name) {
    return testing::MakeMatcher(new SymbolEqMatcher(name));
}
