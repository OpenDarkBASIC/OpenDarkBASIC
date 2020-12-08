#include "odb-compiler/tests/ASTMatchers.hpp"

std::string symbolAnnotationToString(ast::AnnotatedSymbol::Annotation annotation)
{
    using Ann = ast::AnnotatedSymbol::Annotation;
    switch (annotation) {
        default:
        case Ann::NONE : return "NONE";
        case Ann::FLOAT : return "FLOAT";
        case Ann::STRING : return "STRING";
    }
}

template <>
bool LiteralEqMatcher<bool>::MatchAndExplain(const ast::BooleanLiteral* literal, MatchResultListener* listener) const
{
    *listener << "literal->value() equals " << (literal->value() ? "true" : "false");
    return literal->value() == expectedValue_;
}
template <>
void LiteralEqMatcher<bool>::DescribeTo(::std::ostream* os) const {
    *os << "literal->value() equals " << (expectedValue_ ? "true" : "false");
}
template <>
void LiteralEqMatcher<bool>::DescribeNegationTo(::std::ostream* os) const {
    *os << "literal->value() does not equal " << (expectedValue_ ? "true" : "false");
}
