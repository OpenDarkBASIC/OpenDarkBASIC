#include "odb-compiler/tests/matchers/ArrayDeclEq.hpp"
#include "odb-compiler/ast/ArrayDecl.hpp"

ArrayDeclEqMatcher::ArrayDeclEqMatcher(odb::ast::Type type)
    : expectedType_(type)
{}

bool ArrayDeclEqMatcher::MatchAndExplain(const odb::ast::ArrayDecl* node, testing::MatchResultListener* listener) const
{
    *listener
        << "node->type() == " << node->type().toString();
    return node->type() == expectedType_;
}

void ArrayDeclEqMatcher::DescribeTo(::std::ostream* os) const
{
    *os << "node->type() equals " << expectedType_.toString();
}

void ArrayDeclEqMatcher::DescribeNegationTo(::std::ostream* os) const
{
    *os << "node->type() does not equal " << expectedType_.toString();
}
