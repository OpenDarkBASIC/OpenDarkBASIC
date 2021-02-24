#include "odb-compiler/tests/matchers/BinaryOpEq.hpp"
#include "odb-compiler/ast/BinaryOp.hpp"

BinaryOpEqMatcher::BinaryOpEqMatcher(const odb::ast::BinaryOpType op)
    : expectedOp_(op)
{}

bool BinaryOpEqMatcher::MatchAndExplain(const odb::ast::BinaryOp* node, testing::MatchResultListener* listener) const
{
    *listener << "node->op() == " << odb::ast::binaryOpTypeEnumString(node->op());
    return node->op() == expectedOp_;
}

void BinaryOpEqMatcher::DescribeTo(::std::ostream* os) const
{
    *os << "node->op() equals " << odb::ast::binaryOpTypeEnumString(expectedOp_);
}

void BinaryOpEqMatcher::DescribeNegationTo(::std::ostream* os) const
{
    *os << "node->op() does not equal " << odb::ast::binaryOpTypeEnumString(expectedOp_);
}
