#pragma once

#include "gmock/gmock-matchers.h"
#include "odb-compiler/ast/Operators.hpp"

namespace odb::ast {
    class BinaryOp;
}

class BinaryOpEqMatcher : public testing::MatcherInterface<const odb::ast::BinaryOp*>
{
public:
    explicit BinaryOpEqMatcher(const odb::ast::BinaryOpType op);
    bool MatchAndExplain(const odb::ast::BinaryOp* node, testing::MatchResultListener* listener) const override;
    void DescribeTo(::std::ostream* os) const override;
    void DescribeNegationTo(::std::ostream* os) const override;

private:
    const odb::ast::BinaryOpType expectedOp_;
    static const char** table_;
};

inline testing::Matcher<const odb::ast::BinaryOp*> BinaryOpEq(const odb::ast::BinaryOpType op) {
    return testing::MakeMatcher(new BinaryOpEqMatcher(op));
}
