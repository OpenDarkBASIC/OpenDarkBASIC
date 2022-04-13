#pragma once

#include "gmock/gmock-matchers.h"
#include "odb-compiler/ast/Type.hpp"

namespace odb::ast {
    class VarDecl;
}

class VarDeclEqMatcher : public testing::MatcherInterface<const odb::ast::VarDecl*>
{
public:
    explicit VarDeclEqMatcher(odb::ast::Type type);
    bool MatchAndExplain(const odb::ast::VarDecl* node, testing::MatchResultListener* listener) const override;
    void DescribeTo(::std::ostream* os) const override;
    void DescribeNegationTo(::std::ostream* os) const override;

private:
    const odb::ast::Type expectedType_;
};

inline testing::Matcher<const odb::ast::VarDecl*> VarDeclEq(odb::ast::BuiltinType type) {
    return testing::MakeMatcher(new VarDeclEqMatcher(odb::ast::Type::getBuiltin(type)));
}
