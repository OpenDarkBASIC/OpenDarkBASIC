#pragma once

#include "gmock/gmock.h"
#include "odb-compiler/ast/Node.hpp"

using namespace ::testing;
using namespace odb;

std::string symbolScopeToString(ast::Symbol::Scope annotation);
std::string symbolAnnotationToString(ast::Symbol::Annotation annotation);

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

class ExpressionListCountEqMatcher : public MatcherInterface<const ast::ExpressionList*>
{
public:
    explicit ExpressionListCountEqMatcher(int expectedCount) : expectedCount_(expectedCount) {}
    bool MatchAndExplain(const ast::ExpressionList* node, MatchResultListener* listener) const override {
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

class SymbolEqMatcher : public MatcherInterface<const ast::Symbol*>
{
public:
    explicit SymbolEqMatcher(const std::string& name)
        : expectedName_(name) {}
    bool MatchAndExplain(const ast::Symbol* node, MatchResultListener* listener) const override {
        *listener
            << "node->name() == " << node->name();
        return node->name() == expectedName_;
    }
    void DescribeTo(::std::ostream* os) const override {
        *os << "node->name() equals " << expectedName_;
    }
    void DescribeNegationTo(::std::ostream* os) const override {
        *os << "node->name() does not equal " << expectedName_;
    }

private:
    const std::string expectedName_;
};

class AnnotatedSymbolEqMatcher : public MatcherInterface<const ast::AnnotatedSymbol*>
{
public:
    explicit AnnotatedSymbolEqMatcher(ast::Symbol::Annotation annotation, const std::string& name)
        : expectedAnnotation_(annotation)
        , expectedName_(name) {}
    bool MatchAndExplain(const ast::AnnotatedSymbol* node, MatchResultListener* listener) const override {
        *listener
            << "node->annotation() == " << symbolAnnotationToString(node->annotation())
            << ", node->name() == " << node->name();
        return node->annotation() == expectedAnnotation_
            && node->name() == expectedName_;
    }
    void DescribeTo(::std::ostream* os) const override {
        *os
            << "node->annotation() equals " << symbolAnnotationToString(expectedAnnotation_)
            << ", node->name() equals " << expectedName_;
    }
    void DescribeNegationTo(::std::ostream* os) const override {
        *os
            << "node->annotation() does not equal " << symbolAnnotationToString(expectedAnnotation_)
            << "node->name() does not equal " << expectedName_;
    }

private:
    const ast::Symbol::Annotation expectedAnnotation_;
    const std::string expectedName_;
};

class ScopedAnnotatedSymbolEqMatcher : public MatcherInterface<const ast::ScopedAnnotatedSymbol*>
{
public:
    explicit ScopedAnnotatedSymbolEqMatcher(ast::Symbol::Scope scope, ast::Symbol::Annotation annotation, const std::string& name)
        : expectedScope_(scope)
        , expectedAnnotation_(annotation)
        , expectedName_(name) {}
    bool MatchAndExplain(const ast::ScopedAnnotatedSymbol* node, MatchResultListener* listener) const override {
        *listener
            << "node->scope() == " << symbolScopeToString(node->scope())
            << ", node->annotation() == " << symbolAnnotationToString(node->annotation())
            << ", node->name() == " << node->name();
        return node->annotation() == expectedAnnotation_
            && node->name() == expectedName_;
    }
    void DescribeTo(::std::ostream* os) const override {
        *os
            << "node->scope() equals" << symbolScopeToString(expectedScope_)
            << ", node->annotation() equals " << symbolAnnotationToString(expectedAnnotation_)
            << ", node->name() equals " << expectedName_;
    }
    void DescribeNegationTo(::std::ostream* os) const override {
        *os
            << "node->scope() does not equal " << symbolScopeToString(expectedScope_)
            << ", node->annotation() does not equal " << symbolAnnotationToString(expectedAnnotation_)
            << ", node->name() does not equal " << expectedName_;
    }

private:
    const ast::Symbol::Scope expectedScope_;
    const ast::Symbol::Annotation expectedAnnotation_;
    const std::string expectedName_;
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

class KeywordExprSymbolEqMatcher : public MatcherInterface<const ast::KeywordExprSymbol*>
{
public:
    explicit KeywordExprSymbolEqMatcher(const std::string& name)
        : expectedKeyword_(name) {}
    bool MatchAndExplain(const ast::KeywordExprSymbol* node, MatchResultListener* listener) const override {
        *listener << "node->keyword() == " << node->keyword();
        return node->keyword() == expectedKeyword_;
    }
    void DescribeTo(::std::ostream* os) const override {
        *os << "node->keyword() equals " << expectedKeyword_;
    }
    void DescribeNegationTo(::std::ostream* os) const override {
        *os << "node->keyword() does not equal " << expectedKeyword_;
    }

private:
    const std::string expectedKeyword_;
};

class KeywordStmntSymbolEqMatcher : public MatcherInterface<const ast::KeywordStmntSymbol*>
{
public:
    explicit KeywordStmntSymbolEqMatcher(const std::string& name)
        : expectedKeyword_(name) {}
    bool MatchAndExplain(const ast::KeywordStmntSymbol* node, MatchResultListener* listener) const override {
        *listener << "node->keyword() == " << node->keyword();
        return node->keyword() == expectedKeyword_;
    }
    void DescribeTo(::std::ostream* os) const override {
        *os << "node->keyword() equals " << expectedKeyword_;
    }
    void DescribeNegationTo(::std::ostream* os) const override {
        *os << "node->keyword() does not equal " << expectedKeyword_;
    }

private:
    const std::string expectedKeyword_;
};

inline Matcher<const ast::Block*> BlockStmntCountEq(int expectedCount) {
    return MakeMatcher(new BlockStmntCountEqMatcher(expectedCount));
}
inline Matcher<const ast::ExpressionList*> ExpressionListCountEq(int expectedCount) {
    return MakeMatcher(new ExpressionListCountEqMatcher(expectedCount));
}
inline Matcher<const ast::Symbol*> SymbolEq(const std::string& name) {
    return MakeMatcher(new SymbolEqMatcher(name));
}
inline Matcher<const ast::AnnotatedSymbol*> AnnotatedSymbolEq(ast::Symbol::Annotation annotation, const std::string& name) {
    return MakeMatcher(new AnnotatedSymbolEqMatcher(annotation, name));
}
inline Matcher<const ast::ScopedAnnotatedSymbol*> ScopedAnnotatedSymbolEq(ast::Symbol::Scope scope, ast::Symbol::Annotation annotation, const std::string& name) {
    return MakeMatcher(new ScopedAnnotatedSymbolEqMatcher(scope, annotation, name));
}
inline Matcher<const ast::KeywordExprSymbol*> KeywordExprSymbolEq(const std::string& name) {
    return MakeMatcher(new KeywordExprSymbolEqMatcher(name));
}
inline Matcher<const ast::KeywordStmntSymbol*> KeywordStmntSymbolEq(const std::string& name) {
    return MakeMatcher(new KeywordStmntSymbolEqMatcher(name));
}
#define X(dbname, cppname) \
inline Matcher<const ast::dbname##Literal*> dbname##LiteralEq(const cppname& value) { \
    return MakeMatcher(new LiteralEqMatcher<cppname>(value)); \
}
ODB_DATATYPE_LIST
#undef X
