#pragma once

#include "gmock/gmock-matchers.h"
#include "odb-compiler/ast/Type.hpp"

namespace odb::ast {
    class ArrayDecl;
}

class ArrayDeclEqMatcher : public testing::MatcherInterface<const odb::ast::ArrayDecl*>
{
public:
    explicit ArrayDeclEqMatcher(odb::ast::Type type);
    bool MatchAndExplain(const odb::ast::ArrayDecl* node, testing::MatchResultListener* listener) const override;
    void DescribeTo(::std::ostream* os) const override;
    void DescribeNegationTo(::std::ostream* os) const override;

private:
    const odb::ast::Type expectedType_;
};

inline testing::Matcher<const odb::ast::ArrayDecl*> ArrayDeclEq(odb::ast::BuiltinType type) {
    return testing::MakeMatcher(new ArrayDeclEqMatcher(odb::ast::Type::getArray(odb::ast::Type::getBuiltin(type))));
}
