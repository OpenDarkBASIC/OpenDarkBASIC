#pragma once

#include "gmock/gmock-matchers.h"
#include "odb-compiler/ast/Annotation.hpp"

namespace odb::ast {
    class AnnotatedSymbol;
}

class AnnotatedSymbolEqMatcher : public testing::MatcherInterface<const odb::ast::AnnotatedSymbol*>
{
public:
    explicit AnnotatedSymbolEqMatcher(odb::ast::Annotation annotation, const std::string& name);
    bool MatchAndExplain(const odb::ast::AnnotatedSymbol* node, testing::MatchResultListener* listener) const override;
    void DescribeTo(std::ostream* os) const override;
    void DescribeNegationTo(std::ostream* os) const override;

private:
    const odb::ast::Annotation expectedAnnotation_;
    const std::string expectedName_;
};

inline testing::Matcher<const odb::ast::AnnotatedSymbol*> AnnotatedSymbolEq(odb::ast::Annotation annotation, const std::string& name) {
    return MakeMatcher(new AnnotatedSymbolEqMatcher(annotation, name));
}
