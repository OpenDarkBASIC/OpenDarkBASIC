#include "odb-compiler/tests/matchers/UnaryOpEq.hpp"
#include "odb-compiler/ast/UnaryOp.hpp"

UnaryOpEqMatcher::UnaryOpEqMatcher(const odb::ast::UnaryOpType op)
    : expectedOp_(op)
{}

bool UnaryOpEqMatcher::MatchAndExplain(const odb::ast::UnaryOp* node, testing::MatchResultListener* listener) const
{
    *listener << "node->op() == " << unaryOpTypeEnumString(node->op());
    return node->op() == expectedOp_;
}

void UnaryOpEqMatcher::DescribeTo(::std::ostream* os) const
{
    *os << "node->op() equals " << odb::ast::unaryOpTypeEnumString(expectedOp_);
}

void UnaryOpEqMatcher::DescribeNegationTo(::std::ostream* os) const
{
    *os << "node->op() does not equal " << unaryOpTypeEnumString(expectedOp_);
}
