#pragma once

#include "gmock/gmock.h"
#include "odb-compiler/ast/Node.hpp"

using namespace ::testing;
using namespace odb;

std::string symbolAnnotationToString(ast::AnnotatedSymbol::Annotation annotation);

class BlockStmntCountEqMatcher : public MatcherInterface<const ast::Block*>
{
public:
    explicit BlockStmntCountEqMatcher(int expectedCount) : expectedCount_(expectedCount) {}
    bool MatchAndExplain(const ast::Block* block, MatchResultListener* listener) const override {
        *listener << "block->statements().size() equals " << block->statements().size();
        return block->statements().size() == expectedCount_;
    }
    void DescribeTo(::std::ostream* os) const override {
        *os << "block->statements().size() equals " << expectedCount_;
    }
    void DescribeNegationTo(::std::ostream* os) const override {
        *os << "block->statements().size() does not equal " << expectedCount_;
    }

private:
    const int expectedCount_;
};

class ExprListCountEqMatcher : public MatcherInterface<const ast::ExprList*>
{
public:
    explicit ExprListCountEqMatcher(int expectedCount) : expectedCount_(expectedCount) {}
    bool MatchAndExplain(const ast::ExprList* node, MatchResultListener* listener) const override {
        *listener << "node->expressions().size() equals " << node->expressions().size();
        return node->expressions().size() == expectedCount_;
    }
    void DescribeTo(::std::ostream* os) const override {
        *os << "node->expressions().size() equals " << expectedCount_;
    }
    void DescribeNegationTo(::std::ostream* os) const override {
        *os << "node->expressions().size() does not equal " << expectedCount_;
    }

private:
    const int expectedCount_;
};

class AnnotatedSymbolEqMatcher : public MatcherInterface<const ast::AnnotatedSymbol*>
{
public:
    explicit AnnotatedSymbolEqMatcher(const std::string& name, ast::AnnotatedSymbol::Annotation annotation)
        : expectedName_(name)
        , expectedAnnotation_(annotation) {}
    bool MatchAndExplain(const ast::AnnotatedSymbol* node, MatchResultListener* listener) const override {
        *listener
            << "node->name() == " << node->name()
            << ", node->annotation() == " << symbolAnnotationToString(node->annotation());
        return node->name() == expectedName_
            && node->annotation() == expectedAnnotation_;
    }
    void DescribeTo(::std::ostream* os) const override {
        *os << "node->name() equals " << expectedName_
            << ", node->annotation() equals " << symbolAnnotationToString(expectedAnnotation_);
    }
    void DescribeNegationTo(::std::ostream* os) const override {
        *os << "node->name() does not equal " << expectedName_
            << ", node->annotation() does not equal " << symbolAnnotationToString(expectedAnnotation_);
    }

private:
    const std::string expectedName_;
    const ast::AnnotatedSymbol::Annotation expectedAnnotation_;
};

template <class T>
class LiteralEqMatcher : public MatcherInterface<const ast::LiteralTemplate<T>*>
{
public:
    explicit LiteralEqMatcher(const T& expectedValue)
        : expectedValue_(expectedValue) {}
    bool MatchAndExplain(const ast::LiteralTemplate<T>* literal, MatchResultListener* listener) const override {
        *listener << "literal->value() equals " << literal->value();
        return literal->value() == expectedValue_;
    }
    void DescribeTo(::std::ostream* os) const override {
        *os << "literal->value() equals " << expectedValue_;
    }
    void DescribeNegationTo(::std::ostream* os) const override {
        *os << "literal->value() does not equal " << expectedValue_;
    }

private:
    const T expectedValue_;
};

inline Matcher<const ast::Block*> BlockStmntCountEq(int expectedCount) {
    return MakeMatcher(new BlockStmntCountEqMatcher(expectedCount));
}
inline Matcher<const ast::ExprList*> ExprListCountEq(int expectedCount) {
    return MakeMatcher(new ExprListCountEqMatcher(expectedCount));
}
inline Matcher<const ast::AnnotatedSymbol*> AnnotatedSymbolEq(const std::string& name, ast::AnnotatedSymbol::Annotation annotation) {
    return MakeMatcher(new AnnotatedSymbolEqMatcher(name, annotation));
}
#define X(dbname, cppname) \
inline Matcher<const ast::dbname##Literal*> dbname##LiteralEq(const cppname& value) { \
    return MakeMatcher(new LiteralEqMatcher<cppname>(value)); \
}
ODB_DATATYPE_LIST
#undef X
