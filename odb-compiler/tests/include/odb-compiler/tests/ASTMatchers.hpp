#pragma once

#include "gmock/gmock.h"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/ExpressionList.hpp"
#include "odb-compiler/ast/Command.hpp"
#include "odb-compiler/ast/Goto.hpp"
#include "odb-compiler/ast/Label.hpp"
#include "odb-compiler/ast/Literal.hpp"
#include "odb-compiler/ast/Symbol.hpp"
#include "odb-compiler/ast/UDTRef.hpp"

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

class UDTRefEqMatcher : public MatcherInterface<const ast::UDTRef*>
{
public:
    explicit UDTRefEqMatcher(const std::string& name)
        : expectedName_(name) {}
    bool MatchAndExplain(const ast::UDTRef* node, MatchResultListener* listener) const override {
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

class CommandExprSymbolEqMatcher : public MatcherInterface<const ast::CommandExprSymbol*>
{
public:
    explicit CommandExprSymbolEqMatcher(const std::string& name)
        : expectedCommand_(name) {}
    bool MatchAndExplain(const ast::CommandExprSymbol* node, MatchResultListener* listener) const override {
        *listener << "node->command() == " << node->command();
        return node->command() == expectedCommand_;
    }
    void DescribeTo(::std::ostream* os) const override {
        *os << "node->command() equals " << expectedCommand_;
    }
    void DescribeNegationTo(::std::ostream* os) const override {
        *os << "node->command() does not equal " << expectedCommand_;
    }

private:
    const std::string expectedCommand_;
};

class CommandStmntSymbolEqMatcher : public MatcherInterface<const ast::CommandStmntSymbol*>
{
public:
    explicit CommandStmntSymbolEqMatcher(const std::string& name)
        : expectedCommand_(name) {}
    bool MatchAndExplain(const ast::CommandStmntSymbol* node, MatchResultListener* listener) const override {
        *listener << "node->command() == " << node->command();
        return node->command() == expectedCommand_;
    }
    void DescribeTo(::std::ostream* os) const override {
        *os << "node->command() equals " << expectedCommand_;
    }
    void DescribeNegationTo(::std::ostream* os) const override {
        *os << "node->command() does not equal " << expectedCommand_;
    }

private:
    const std::string expectedCommand_;
};

class GotoEqMatcher : public MatcherInterface<const ast::Goto*>
{
public:
    explicit GotoEqMatcher(const std::string& labelName)
        : expectedLabel_(labelName) {}
    bool MatchAndExplain(const ast::Goto* node, MatchResultListener* listener) const override {
        *listener << "node->label()->symbol()->name() == " << node->label()->symbol()->name();
        return node->label()->symbol()->name() == expectedLabel_;
    }
    void DescribeTo(::std::ostream* os) const override {
        *os << "node->label()->symbol()->name() equals " << expectedLabel_;
    }
    void DescribeNegationTo(::std::ostream* os) const override {
        *os << "node->label()->symbol()->name() does not equal " << expectedLabel_;
    }

private:
    const std::string expectedLabel_;
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
inline Matcher<const ast::UDTRef*> UDTRefEq(const std::string& name) {
    return MakeMatcher(new UDTRefEqMatcher(name));
}
inline Matcher<const ast::CommandExprSymbol*> CommandExprSymbolEq(const std::string& name) {
    return MakeMatcher(new CommandExprSymbolEqMatcher(name));
}
inline Matcher<const ast::CommandStmntSymbol*> CommandStmntSymbolEq(const std::string& name) {
    return MakeMatcher(new CommandStmntSymbolEqMatcher(name));
}
inline Matcher<const ast::Goto*> GotoEq(const std::string& labelName) {
    return MakeMatcher(new GotoEqMatcher(labelName));
}
#define X(dbname, cppname) \
inline Matcher<const ast::dbname##Literal*> dbname##LiteralEq(const cppname& value) { \
    return MakeMatcher(new LiteralEqMatcher<cppname>(value)); \
}
ODB_DATATYPE_LIST
#undef X
