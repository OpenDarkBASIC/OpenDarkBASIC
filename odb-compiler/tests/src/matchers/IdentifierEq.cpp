#include <utility>

#include "odb-compiler/ast/Identifier.hpp"
#include "odb-compiler/tests/matchers/IdentifierEq.hpp"

// ----------------------------------------------------------------------------
IdentifierEqMatcher::IdentifierEqMatcher(std::string name, odb::ast::Annotation annotation)
    : expectedName_(std::move(name))
    , expectedAnnotation_(annotation)
{}

// ----------------------------------------------------------------------------
bool IdentifierEqMatcher::MatchAndExplain(const odb::ast::Identifier* node, testing::MatchResultListener* listener) const
{
    *listener
        << "node->annotation() == " << odb::ast::typeAnnotationEnumString(node->annotation())
        << ", node->name() == " << node->name();
    return node->annotation() == expectedAnnotation_
        && node->name() == expectedName_;
}

// ----------------------------------------------------------------------------
void IdentifierEqMatcher::DescribeTo(std::ostream* os) const
{
    *os << "node->annotation() equals " << odb::ast::typeAnnotationEnumString(expectedAnnotation_)
        << ", node->name() equals " << expectedName_;
}

// ----------------------------------------------------------------------------
void IdentifierEqMatcher::DescribeNegationTo(std::ostream* os) const
{
    *os << "node->annotation() does not equal " << odb::ast::typeAnnotationEnumString(expectedAnnotation_)
        << "node->name() does not equal " << expectedName_;
}
