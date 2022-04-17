#include <utility>

#include "odb-compiler/ast/ScopedIdentifier.hpp"
#include "odb-compiler/tests/matchers/ScopedIdentifierEq.hpp"

ScopedIdentifierEqMatcher::ScopedIdentifierEqMatcher(odb::ast::Scope scope, std::string name, odb::ast::Annotation annotation)
    : expectedScope_(scope)
    , expectedName_(std::move(name))
    , expectedAnnotation_(annotation)
{}

bool ScopedIdentifierEqMatcher::MatchAndExplain(const odb::ast::ScopedIdentifier* node, testing::MatchResultListener* listener) const
{
    *listener
        << "node->scope() == " << odb::ast::scopeEnumString(node->scope())
        << ", node->annotation() == " << odb::ast::typeAnnotationEnumString(node->annotation())
        << ", node->name() == " << node->name();
    return node->annotation() == expectedAnnotation_
        && node->name() == expectedName_;
}

void ScopedIdentifierEqMatcher::DescribeTo(::std::ostream* os) const
{
    *os
        << "node->scope() equals" << odb::ast::scopeEnumString(expectedScope_)
        << ", node->annotation() equals " << odb::ast::typeAnnotationEnumString(expectedAnnotation_)
        << ", node->name() equals " << expectedName_;
}

void ScopedIdentifierEqMatcher::DescribeNegationTo(::std::ostream* os) const
{
    *os
        << "node->scope() does not equal " << odb::ast::scopeEnumString(expectedScope_)
        << ", node->annotation() does not equal " << odb::ast::typeAnnotationEnumString(expectedAnnotation_)
        << ", node->name() does not equal " << expectedName_;
}
