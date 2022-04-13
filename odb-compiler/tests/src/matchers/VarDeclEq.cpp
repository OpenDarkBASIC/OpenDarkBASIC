#include "odb-compiler/tests/matchers/VarDeclEq.hpp"
#include "odb-compiler/ast/VarDecl.hpp"

VarDeclEqMatcher::VarDeclEqMatcher(odb::ast::Type type)
    : expectedType_(type)
{}

bool VarDeclEqMatcher::MatchAndExplain(const odb::ast::VarDecl* node, testing::MatchResultListener* listener) const
{
    *listener
        << "node->type() == " << node->type().toString();
    return node->type() == expectedType_;
}

void VarDeclEqMatcher::DescribeTo(::std::ostream* os) const
{
    *os << "node->type() equals " << expectedType_.toString();
}

void VarDeclEqMatcher::DescribeNegationTo(::std::ostream* os) const
{
    *os << "node->type() does not equal " << expectedType_.toString();
}
