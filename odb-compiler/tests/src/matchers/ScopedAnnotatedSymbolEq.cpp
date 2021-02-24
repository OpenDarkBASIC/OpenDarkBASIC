#include "odb-compiler/tests/matchers/ScopedAnnotatedSymbolEq.hpp"
#include "odb-compiler/ast/ScopedAnnotatedSymbol.hpp"

ScopedAnnotatedSymbolEqMatcher::ScopedAnnotatedSymbolEqMatcher(odb::ast::Scope scope, odb::ast::Annotation annotation, const std::string& name)
    : expectedScope_(scope)
    , expectedAnnotation_(annotation)
    , expectedName_(name)
{}

bool ScopedAnnotatedSymbolEqMatcher::MatchAndExplain(const odb::ast::ScopedAnnotatedSymbol* node, testing::MatchResultListener* listener) const
{
    *listener
        << "node->scope() == " << odb::ast::scopeEnumString(node->scope())
        << ", node->annotation() == " << odb::ast::typeAnnotationEnumString(node->annotation())
        << ", node->name() == " << node->name();
    return node->annotation() == expectedAnnotation_
        && node->name() == expectedName_;
}

void ScopedAnnotatedSymbolEqMatcher::DescribeTo(::std::ostream* os) const
{
    *os
        << "node->scope() equals" << odb::ast::scopeEnumString(expectedScope_)
        << ", node->annotation() equals " << odb::ast::typeAnnotationEnumString(expectedAnnotation_)
        << ", node->name() equals " << expectedName_;
}

void ScopedAnnotatedSymbolEqMatcher::DescribeNegationTo(::std::ostream* os) const
{
    *os
        << "node->scope() does not equal " << odb::ast::scopeEnumString(expectedScope_)
        << ", node->annotation() does not equal " << odb::ast::typeAnnotationEnumString(expectedAnnotation_)
        << ", node->name() does not equal " << expectedName_;
}
