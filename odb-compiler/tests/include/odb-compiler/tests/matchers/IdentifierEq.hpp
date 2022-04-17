#pragma once

#include "gmock/gmock-matchers.h"
#include "odb-compiler/ast/Annotation.hpp"

namespace odb::ast {
    class Identifier;
}

class IdentifierEqMatcher : public testing::MatcherInterface<const odb::ast::Identifier*>
{
public:
    explicit IdentifierEqMatcher(std::string name, odb::ast::Annotation annotation);
    bool MatchAndExplain(const odb::ast::Identifier* node, testing::MatchResultListener* listener) const override;
    void DescribeTo(std::ostream* os) const override;
    void DescribeNegationTo(std::ostream* os) const override;

private:
    const std::string expectedName_;
    const odb::ast::Annotation expectedAnnotation_;
};

inline testing::Matcher<const odb::ast::Identifier*>
IdentifierEq(std::string name, odb::ast::Annotation annotation = odb::ast::Annotation::NONE)
{
    return MakeMatcher(new IdentifierEqMatcher(std::move(name), annotation));
}
