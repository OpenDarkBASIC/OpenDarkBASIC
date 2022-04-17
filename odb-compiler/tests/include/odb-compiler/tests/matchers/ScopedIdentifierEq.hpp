#pragma once

#include "gmock/gmock-matchers.h"
#include "odb-compiler/ast/Scope.hpp"
#include "odb-compiler/ast/Annotation.hpp"

namespace odb::ast {
    class ScopedIdentifier;
}

class ScopedIdentifierEqMatcher : public testing::MatcherInterface<const odb::ast::ScopedIdentifier*>
{
public:
    explicit ScopedIdentifierEqMatcher(odb::ast::Scope scope, std::string name, odb::ast::Annotation annotation);
    bool MatchAndExplain(const odb::ast::ScopedIdentifier* node, testing::MatchResultListener* listener) const override;
    void DescribeTo(::std::ostream* os) const override;
    void DescribeNegationTo(::std::ostream* os) const override;

private:
    const std::string expectedName_;
    const odb::ast::Scope expectedScope_;
    const odb::ast::Annotation expectedAnnotation_;
};

inline testing::Matcher<const odb::ast::ScopedIdentifier*>
ScopedIdentifierEq(odb::ast::Scope scope, std::string name, odb::ast::Annotation annotation) {
    return testing::MakeMatcher(new ScopedIdentifierEqMatcher(scope, std::move(name), annotation));
}
