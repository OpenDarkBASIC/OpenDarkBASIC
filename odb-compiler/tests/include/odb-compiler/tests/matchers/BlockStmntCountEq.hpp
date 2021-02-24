#pragma once

#include "gmock/gmock-matchers.h"

namespace odb::ast {
    class Block;
}

class BlockStmntCountEqMatcher : public testing::MatcherInterface<const odb::ast::Block*>
{
public:
    explicit BlockStmntCountEqMatcher(int expectedCount);
    bool MatchAndExplain(const odb::ast::Block* block, testing::MatchResultListener* listener) const override;
    void DescribeTo(::std::ostream* os) const override;
    void DescribeNegationTo(::std::ostream* os) const override;

private:
    const int expectedCount_;
};

inline testing::Matcher<const odb::ast::Block*> BlockStmntCountEq(int expectedCount) {
    return MakeMatcher(new BlockStmntCountEqMatcher(expectedCount));
}
