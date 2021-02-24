#include "odb-compiler/tests/matchers/SymbolEq.hpp"
#include "odb-compiler/ast/Symbol.hpp"

SymbolEqMatcher::SymbolEqMatcher(const std::string& name)
    : expectedName_(name)
{}

bool SymbolEqMatcher::MatchAndExplain(const odb::ast::Symbol* node, testing::MatchResultListener* listener) const
{
    *listener
        << "node->name() == " << node->name();
    return node->name() == expectedName_;
}

void SymbolEqMatcher::DescribeTo(::std::ostream* os) const
{
    *os << "node->name() equals " << expectedName_;
}

void SymbolEqMatcher::DescribeNegationTo(::std::ostream* os) const
{
    *os << "node->name() does not equal " << expectedName_;
}
