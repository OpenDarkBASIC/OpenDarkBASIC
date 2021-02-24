#include "odb-compiler/tests/matchers/AnnotatedSymbolEq.hpp"
#include "odb-compiler/ast/AnnotatedSymbol.hpp"

// ----------------------------------------------------------------------------
AnnotatedSymbolEqMatcher::AnnotatedSymbolEqMatcher(odb::ast::Annotation annotation, const std::string& name)
    : expectedAnnotation_(annotation)
    , expectedName_(name)
{}

// ----------------------------------------------------------------------------
bool AnnotatedSymbolEqMatcher::MatchAndExplain(const odb::ast::AnnotatedSymbol* node, testing::MatchResultListener* listener) const
{
    *listener
        << "node->annotation() == " << odb::ast::typeAnnotationEnumString(node->annotation())
        << ", node->name() == " << node->name();
    return node->annotation() == expectedAnnotation_
        && node->name() == expectedName_;
}

// ----------------------------------------------------------------------------
void AnnotatedSymbolEqMatcher::DescribeTo(std::ostream* os) const
{
    *os << "node->annotation() equals " << odb::ast::typeAnnotationEnumString(expectedAnnotation_)
        << ", node->name() equals " << expectedName_;
}

// ----------------------------------------------------------------------------
void AnnotatedSymbolEqMatcher::DescribeNegationTo(std::ostream* os) const
{
    *os << "node->annotation() does not equal " << odb::ast::typeAnnotationEnumString(expectedAnnotation_)
        << "node->name() does not equal " << expectedName_;
}
