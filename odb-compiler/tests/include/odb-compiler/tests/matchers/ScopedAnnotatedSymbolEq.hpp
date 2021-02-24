#pragma once

#include "gmock/gmock-matchers.h"
#include "odb-compiler/ast/Scope.hpp"
#include "odb-compiler/ast/Annotation.hpp"

namespace odb::ast {
    class ScopedAnnotatedSymbol;
}

class ScopedAnnotatedSymbolEqMatcher : public testing::MatcherInterface<const odb::ast::ScopedAnnotatedSymbol*>
{
public:
    explicit ScopedAnnotatedSymbolEqMatcher(odb::ast::Scope scope, odb::ast::Annotation annotation, const std::string& name);
    bool MatchAndExplain(const odb::ast::ScopedAnnotatedSymbol* node, testing::MatchResultListener* listener) const override;
    void DescribeTo(::std::ostream* os) const override;
    void DescribeNegationTo(::std::ostream* os) const override;

private:
    const odb::ast::Scope expectedScope_;
    const odb::ast::Annotation expectedAnnotation_;
    const std::string expectedName_;
};

inline testing::Matcher<const odb::ast::ScopedAnnotatedSymbol*> ScopedAnnotatedSymbolEq(odb::ast::Scope scope, odb::ast::Annotation annotation, const std::string& name) {
    return testing::MakeMatcher(new ScopedAnnotatedSymbolEqMatcher(scope, annotation, name));
}
