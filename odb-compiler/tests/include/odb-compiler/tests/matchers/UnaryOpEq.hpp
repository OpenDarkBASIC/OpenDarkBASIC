#pragma once

#include "gmock/gmock-matchers.h"
#include "odb-compiler/ast/Operators.hpp"

namespace odb::ast {
    class UnaryOp;
}

class UnaryOpEqMatcher : public testing::MatcherInterface<const odb::ast::UnaryOp*>
{
public:
    explicit UnaryOpEqMatcher(const odb::ast::UnaryOpType op);
    bool MatchAndExplain(const odb::ast::UnaryOp* node, testing::MatchResultListener* listener) const override;
    void DescribeTo(::std::ostream* os) const override;
    void DescribeNegationTo(::std::ostream* os) const override;

private:
    const odb::ast::UnaryOpType expectedOp_;
    static const char** table_;
};

inline testing::Matcher<const odb::ast::UnaryOp*> UnaryOpEq(const odb::ast::UnaryOpType op) {
    return testing::MakeMatcher(new UnaryOpEqMatcher(op));
}
