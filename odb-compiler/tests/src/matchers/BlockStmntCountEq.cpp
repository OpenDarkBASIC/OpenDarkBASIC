#include "odb-compiler/tests/matchers/BlockStmntCountEq.hpp"
#include "odb-compiler/ast/Block.hpp"

BlockStmntCountEqMatcher::BlockStmntCountEqMatcher(int expectedCount)
    : expectedCount_(expectedCount)
{}

bool BlockStmntCountEqMatcher::MatchAndExplain(const odb::ast::Block* block, testing::MatchResultListener* listener) const {
    *listener << "block->statements().size() equals " << block->statements().size();
    return block->statements().size() == expectedCount_;
}

void BlockStmntCountEqMatcher::DescribeTo(::std::ostream* os) const {
    *os << "block->statements().size() equals " << expectedCount_;
}

void BlockStmntCountEqMatcher::DescribeNegationTo(::std::ostream* os) const {
    *os << "block->statements().size() does not equal " << expectedCount_;
}
