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

template <>
class LiteralEqMatcher<odb::Complex<float>> : public MatcherInterface<const ast::LiteralTemplate<odb::Complex<float>>*>
{
public:
    explicit LiteralEqMatcher(const odb::Complex<float>& expectedValue)
        : expectedValue_(expectedValue) {}
    bool MatchAndExplain(const ast::LiteralTemplate<odb::Complex<float>>* literal, MatchResultListener* listener) const override {
        *listener << "literal->value() equals " << literal->value().real << " + " << literal->value().imag << "i";
        return literal->value().real == expectedValue_.real && literal->value().imag == expectedValue_.imag;
    }
    void DescribeTo(::std::ostream* os) const override {
        *os << "literal->value() equals " << expectedValue_.real << " + " << expectedValue_.imag << "i";
    }
    void DescribeNegationTo(::std::ostream* os) const override {
        *os << "literal->value() does not equal " << expectedValue_.real << " + " << expectedValue_.imag << "i";
    }
private:
    const odb::Complex<float> expectedValue_;
};

template <>
class LiteralEqMatcher<odb::Quat<float>> : public MatcherInterface<const ast::LiteralTemplate<odb::Quat<float>>*>
{
public:
    explicit LiteralEqMatcher(const odb::Quat<float>& expectedValue)
        : expectedValue_(expectedValue) {}
    bool MatchAndExplain(const ast::LiteralTemplate<odb::Quat<float>>* literal, MatchResultListener* listener) const override {
        *listener << "literal->value() equals " << literal->value().r << " + " << literal->value().i << "i + " << literal->value().j << "j + " << literal->value().k << "k";
        return literal->value().i == expectedValue_.i && literal->value().j == expectedValue_.j && literal->value().k == expectedValue_.k && literal->value().r == expectedValue_.r;
    }
    void DescribeTo(::std::ostream* os) const override {
        *os << "literal->value() equals " << expectedValue_.r << " + " << expectedValue_.i << "i + " << expectedValue_.j << "j + " << expectedValue_.k << "k";
    }
    void DescribeNegationTo(::std::ostream* os) const override {
        *os << "literal->value() does not equal " << expectedValue_.r << " + " << expectedValue_.i << "i + " << expectedValue_.j << "j + " << expectedValue_.k << "k";
    }
private:
    const odb::Quat<float> expectedValue_;
};

template <>
class LiteralEqMatcher<odb::Vec2<float>> : public MatcherInterface<const ast::LiteralTemplate<odb::Vec2<float>>*>
{
public:
    explicit LiteralEqMatcher(const odb::Vec2<float>& expectedValue)
        : expectedValue_(expectedValue) {}
    bool MatchAndExplain(const ast::LiteralTemplate<odb::Vec2<float>>* literal, MatchResultListener* listener) const override {
        *listener << "literal->value() equals [" << literal->value().x << ", " << literal->value().y << "]";
        return literal->value().x == expectedValue_.x && literal->value().y == expectedValue_.y;
    }
    void DescribeTo(::std::ostream* os) const override {
        *os << "literal->value() equals [" << expectedValue_.x << ", " << expectedValue_.y << "]";
    }
    void DescribeNegationTo(::std::ostream* os) const override {
        *os << "literal->value() does not equal [" << expectedValue_.x << ", " << expectedValue_.y << "]";
    }
private:
    const odb::Vec2<float> expectedValue_;
};

template <>
class LiteralEqMatcher<odb::Vec3<float>> : public MatcherInterface<const ast::LiteralTemplate<odb::Vec3<float>>*>
{
public:
    explicit LiteralEqMatcher(const odb::Vec3<float>& expectedValue)
        : expectedValue_(expectedValue) {}
    bool MatchAndExplain(const ast::LiteralTemplate<odb::Vec3<float>>* literal, MatchResultListener* listener) const override {
        *listener << "literal->value() equals [" << literal->value().x << ", " << literal->value().y << ", " << literal->value().z << "]";
        return literal->value().x == expectedValue_.x && literal->value().y == expectedValue_.y && literal->value().z == expectedValue_.z;
    }
    void DescribeTo(::std::ostream* os) const override {
        *os << "literal->value() equals [" << expectedValue_.x << ", " << expectedValue_.y << ", " << expectedValue_.z << "]";
    }
    void DescribeNegationTo(::std::ostream* os) const override {
        *os << "literal->value() does not equal [" << expectedValue_.x << ", " << expectedValue_.y << ", " << expectedValue_.z << "]";
    }
private:
    const odb::Vec3<float> expectedValue_;
};

template <>
class LiteralEqMatcher<odb::Vec4<float>> : public MatcherInterface<const ast::LiteralTemplate<odb::Vec4<float>>*>
{
public:
    explicit LiteralEqMatcher(const odb::Vec4<float>& expectedValue)
        : expectedValue_(expectedValue) {}
    bool MatchAndExplain(const ast::LiteralTemplate<odb::Vec4<float>>* literal, MatchResultListener* listener) const override {
        *listener << "literal->value() equals [" << literal->value().x << ", " << literal->value().y << ", " << literal->value().z << ", " << literal->value().w << "]";
        return literal->value().x == expectedValue_.x && literal->value().y == expectedValue_.y && literal->value().z == expectedValue_.z && literal->value().w == expectedValue_.w;
    }
    void DescribeTo(::std::ostream* os) const override {
        *os << "literal->value() equals [" << expectedValue_.x << ", " << expectedValue_.y << ", " << expectedValue_.z << ", " << expectedValue_.w << "]";
    }
    void DescribeNegationTo(::std::ostream* os) const override {
        *os << "literal->value() does not equal [" << expectedValue_.x << ", " << expectedValue_.y << ", " << expectedValue_.z << ", " << expectedValue_.w << "]";
    }
private:
    const odb::Vec4<float> expectedValue_;
};

template <>
class LiteralEqMatcher<odb::Mat2x2<float>> : public MatcherInterface<const ast::LiteralTemplate<odb::Mat2x2<float>>*>
{
public:
    explicit LiteralEqMatcher(const odb::Mat2x2<float>& expectedValue)
        : expectedValue_(expectedValue) {}
    bool MatchAndExplain(const ast::LiteralTemplate<odb::Mat2x2<float>>* literal, MatchResultListener* listener) const override {
        *listener << "literal->value() equals [" << literal->value().e0.x << ", " << literal->value().e1.x << ";\n"
                  << "                         " << literal->value().e0.y << ", " << literal->value().e1.y << "]";
        return literal->value().e0.x == expectedValue_.e0.x && literal->value().e1.x == expectedValue_.e1.x
            && literal->value().e0.y == expectedValue_.e0.y && literal->value().e1.y == expectedValue_.e1.y;
    }
    void DescribeTo(::std::ostream* os) const override {
        *os << "literal->value() equals [" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ";\n"
            << "                         " << expectedValue_.e0.y << ", " << expectedValue_.e1.y << "]";
    }
    void DescribeNegationTo(::std::ostream* os) const override {
        *os << "literal->value() does not equal [" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ";\n"
            << "                                 " << expectedValue_.e0.y << ", " << expectedValue_.e1.y << "]";
    }
private:
    const odb::Mat2x2<float> expectedValue_;
};

template <>
class LiteralEqMatcher<odb::Mat2x3<float>> : public MatcherInterface<const ast::LiteralTemplate<odb::Mat2x3<float>>*>
{
public:
    explicit LiteralEqMatcher(const odb::Mat2x3<float>& expectedValue)
        : expectedValue_(expectedValue) {}
    bool MatchAndExplain(const ast::LiteralTemplate<odb::Mat2x3<float>>* literal, MatchResultListener* listener) const override {
        *listener << "literal->value() equals [" << literal->value().e0.x << ", " << literal->value().e1.x << ";\n"
                  << "                         " << literal->value().e0.y << ", " << literal->value().e1.y << "]";
        return literal->value().e0.x == expectedValue_.e0.x && literal->value().e1.x == expectedValue_.e1.x && literal->value().e2.x == expectedValue_.e2.x
            && literal->value().e0.y == expectedValue_.e0.y && literal->value().e1.y == expectedValue_.e1.y && literal->value().e2.y == expectedValue_.e2.y;
    }
    void DescribeTo(::std::ostream* os) const override {
        *os << "literal->value() equals [" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ", " << expectedValue_.e2.x << ";\n"
            << "                         " << expectedValue_.e0.y << ", " << expectedValue_.e1.y << ", " << expectedValue_.e2.y << "]";
    }
    void DescribeNegationTo(::std::ostream* os) const override {
        *os << "literal->value() does not equal [" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ", " << expectedValue_.e2.x << ";\n"
            << "                                 " << expectedValue_.e0.y << ", " << expectedValue_.e1.y << ", " << expectedValue_.e2.y << "]";
    }
private:
    const odb::Mat2x3<float> expectedValue_;
};

template <>
class LiteralEqMatcher<odb::Mat2x4<float>> : public MatcherInterface<const ast::LiteralTemplate<odb::Mat2x4<float>>*>
{
public:
    explicit LiteralEqMatcher(const odb::Mat2x4<float>& expectedValue)
        : expectedValue_(expectedValue) {}
    bool MatchAndExplain(const ast::LiteralTemplate<odb::Mat2x4<float>>* literal, MatchResultListener* listener) const override {
        *listener << "literal->value() equals [" << literal->value().e0.x << ", " << literal->value().e1.x << ", " << literal->value().e2.x << ", " << literal->value().e3.x << ";\n"
                  << "                         " << literal->value().e0.y << ", " << literal->value().e1.y << ", " << literal->value().e2.y << ", " << literal->value().e3.y << "]";
        return literal->value().e0.x == expectedValue_.e0.x && literal->value().e1.x == expectedValue_.e1.x && literal->value().e2.x == expectedValue_.e2.x && literal->value().e3.x == expectedValue_.e3.x
            && literal->value().e0.y == expectedValue_.e0.y && literal->value().e1.y == expectedValue_.e1.y && literal->value().e2.y == expectedValue_.e2.y && literal->value().e3.y == expectedValue_.e3.y;
    }
    void DescribeTo(::std::ostream* os) const override {
        *os << "literal->value() equals [" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ", " << expectedValue_.e2.x << ", " << expectedValue_.e3.x << ";\n"
            << "                         " << expectedValue_.e0.y << ", " << expectedValue_.e1.y << ", " << expectedValue_.e2.y << ", " << expectedValue_.e3.y << "]";
    }
    void DescribeNegationTo(::std::ostream* os) const override {
        *os << "literal->value() does not equal [" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ", " << expectedValue_.e2.x << ", " << expectedValue_.e3.x << ";\n"
            << "                                 " << expectedValue_.e0.y << ", " << expectedValue_.e1.y << ", " << expectedValue_.e2.y << ", " << expectedValue_.e3.y << "]";
    }
private:
    const odb::Mat2x4<float> expectedValue_;
};

template <>
class LiteralEqMatcher<odb::Mat3x2<float>> : public MatcherInterface<const ast::LiteralTemplate<odb::Mat3x2<float>>*>
{
public:
    explicit LiteralEqMatcher(const odb::Mat3x2<float>& expectedValue)
        : expectedValue_(expectedValue) {}
    bool MatchAndExplain(const ast::LiteralTemplate<odb::Mat3x2<float>>* literal, MatchResultListener* listener) const override {
        *listener << "literal->value() equals [" << literal->value().e0.x << ", " << literal->value().e1.x << ";\n"
                  << "                         " << literal->value().e0.y << ", " << literal->value().e1.y << ";\n"
                  << "                         " << literal->value().e0.z << ", " << literal->value().e1.z << "]";
        return literal->value().e0.x == expectedValue_.e0.x && literal->value().e1.x == expectedValue_.e1.x
            && literal->value().e0.y == expectedValue_.e0.y && literal->value().e1.y == expectedValue_.e1.y
            && literal->value().e0.z == expectedValue_.e0.z && literal->value().e1.z == expectedValue_.e1.z;
    }
    void DescribeTo(::std::ostream* os) const override {
        *os << "literal->value() equals [" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ";\n"
            << "                         " << expectedValue_.e0.y << ", " << expectedValue_.e1.y << ";\n"
            << "                         " << expectedValue_.e0.z << ", " << expectedValue_.e1.z << "]";
    }
    void DescribeNegationTo(::std::ostream* os) const override {
        *os << "literal->value() does not equal [" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ";\n"
            << "                                 " << expectedValue_.e0.y << ", " << expectedValue_.e1.y << ";\n"
            << "                                 " << expectedValue_.e0.z << ", " << expectedValue_.e1.z << "]";
    }
private:
    const odb::Mat3x2<float> expectedValue_;
};

template <>
class LiteralEqMatcher<odb::Mat3x3<float>> : public MatcherInterface<const ast::LiteralTemplate<odb::Mat3x3<float>>*>
{
public:
    explicit LiteralEqMatcher(const odb::Mat3x3<float>& expectedValue)
        : expectedValue_(expectedValue) {}
    bool MatchAndExplain(const ast::LiteralTemplate<odb::Mat3x3<float>>* literal, MatchResultListener* listener) const override {
        *listener << "literal->value() equals [" << literal->value().e0.x << ", " << literal->value().e1.x << ";\n"
                  << "                         " << literal->value().e0.y << ", " << literal->value().e1.y << ";\n"
                  << "                         " << literal->value().e0.z << ", " << literal->value().e1.z << "]";
        return literal->value().e0.x == expectedValue_.e0.x && literal->value().e1.x == expectedValue_.e1.x && literal->value().e2.x == expectedValue_.e2.x
            && literal->value().e0.y == expectedValue_.e0.y && literal->value().e1.y == expectedValue_.e1.y && literal->value().e2.y == expectedValue_.e2.y
            && literal->value().e0.z == expectedValue_.e0.z && literal->value().e1.z == expectedValue_.e1.z && literal->value().e2.z == expectedValue_.e2.z;
    }
    void DescribeTo(::std::ostream* os) const override {
        *os << "literal->value() equals [" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ", " << expectedValue_.e2.x << ";\n"
            << "                         " << expectedValue_.e0.y << ", " << expectedValue_.e1.y << ", " << expectedValue_.e2.y << ";\n"
            << "                         " << expectedValue_.e0.z << ", " << expectedValue_.e1.z << ", " << expectedValue_.e2.z << "]";
    }
    void DescribeNegationTo(::std::ostream* os) const override {
        *os << "literal->value() does not equal [" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ", " << expectedValue_.e2.x << ";\n"
            << "                                 " << expectedValue_.e0.y << ", " << expectedValue_.e1.y << ", " << expectedValue_.e2.y << ";\n"
            << "                                 " << expectedValue_.e0.z << ", " << expectedValue_.e1.z << ", " << expectedValue_.e2.z << "]";
    }
private:
    const odb::Mat3x3<float> expectedValue_;
};

template <>
class LiteralEqMatcher<odb::Mat3x4<float>> : public MatcherInterface<const ast::LiteralTemplate<odb::Mat3x4<float>>*>
{
public:
    explicit LiteralEqMatcher(const odb::Mat3x4<float>& expectedValue)
        : expectedValue_(expectedValue) {}
    bool MatchAndExplain(const ast::LiteralTemplate<odb::Mat3x4<float>>* literal, MatchResultListener* listener) const override {
        *listener << "literal->value() equals [" << literal->value().e0.x << ", " << literal->value().e1.x << ", " << literal->value().e2.x << ", " << literal->value().e3.x << ";\n"
                  << "                         " << literal->value().e0.y << ", " << literal->value().e1.y << ", " << literal->value().e2.y << ", " << literal->value().e3.y << ";\n"
                  << "                         " << literal->value().e0.z << ", " << literal->value().e1.z << ", " << literal->value().e2.z << ", " << literal->value().e3.z << "]";
        return literal->value().e0.x == expectedValue_.e0.x && literal->value().e1.x == expectedValue_.e1.x && literal->value().e2.x == expectedValue_.e2.x && literal->value().e3.x == expectedValue_.e3.x
            && literal->value().e0.y == expectedValue_.e0.y && literal->value().e1.y == expectedValue_.e1.y && literal->value().e2.y == expectedValue_.e2.y && literal->value().e3.y == expectedValue_.e3.y
            && literal->value().e0.z == expectedValue_.e0.z && literal->value().e1.z == expectedValue_.e1.z && literal->value().e2.z == expectedValue_.e2.z && literal->value().e3.z == expectedValue_.e3.z;
    }
    void DescribeTo(::std::ostream* os) const override {
        *os << "literal->value() equals [" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ", " << expectedValue_.e2.x << ", " << expectedValue_.e3.x << ";\n"
            << "                         " << expectedValue_.e0.y << ", " << expectedValue_.e1.y << ", " << expectedValue_.e2.y << ", " << expectedValue_.e3.y << ";\n"
            << "                         " << expectedValue_.e0.z << ", " << expectedValue_.e1.z << ", " << expectedValue_.e2.z << ", " << expectedValue_.e3.z << "]";
    }
    void DescribeNegationTo(::std::ostream* os) const override {
        *os << "literal->value() does not equal [" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ", " << expectedValue_.e2.x << ", " << expectedValue_.e3.x << ";\n"
            << "                                 " << expectedValue_.e0.y << ", " << expectedValue_.e1.y << ", " << expectedValue_.e2.y << ", " << expectedValue_.e3.y << ";\n"
            << "                                 " << expectedValue_.e0.z << ", " << expectedValue_.e1.z << ", " << expectedValue_.e2.z << ", " << expectedValue_.e3.z << "]";
    }
private:
    const odb::Mat3x4<float> expectedValue_;
};

template <>
class LiteralEqMatcher<odb::Mat4x2<float>> : public MatcherInterface<const ast::LiteralTemplate<odb::Mat4x2<float>>*>
{
public:
    explicit LiteralEqMatcher(const odb::Mat4x2<float>& expectedValue)
        : expectedValue_(expectedValue) {}
    bool MatchAndExplain(const ast::LiteralTemplate<odb::Mat4x2<float>>* literal, MatchResultListener* listener) const override {
        *listener << "literal->value() equals [" << literal->value().e0.x << ", " << literal->value().e1.x << ";\n"
                  << "                         " << literal->value().e0.y << ", " << literal->value().e1.y << ";\n"
                  << "                         " << literal->value().e0.z << ", " << literal->value().e1.z << ";\n"
                  << "                         " << literal->value().e0.w << ", " << literal->value().e1.w << "]";
        return literal->value().e0.x == expectedValue_.e0.x && literal->value().e1.x == expectedValue_.e1.x
            && literal->value().e0.y == expectedValue_.e0.y && literal->value().e1.y == expectedValue_.e1.y
            && literal->value().e0.w == expectedValue_.e0.z && literal->value().e1.z == expectedValue_.e1.z
            && literal->value().e0.z == expectedValue_.e0.w && literal->value().e1.w == expectedValue_.e1.w;
    }
    void DescribeTo(::std::ostream* os) const override {
        *os << "literal->value() equals [" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ";\n"
            << "                         " << expectedValue_.e0.y << ", " << expectedValue_.e1.y << ";\n"
            << "                         " << expectedValue_.e0.z << ", " << expectedValue_.e1.z << ";\n"
            << "                         " << expectedValue_.e0.w << ", " << expectedValue_.e1.w << "]";
    }
    void DescribeNegationTo(::std::ostream* os) const override {
        *os << "literal->value() does not equal [" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ";\n"
            << "                                 " << expectedValue_.e0.y << ", " << expectedValue_.e1.y << ";\n"
            << "                                 " << expectedValue_.e0.z << ", " << expectedValue_.e1.z << ";\n"
            << "                                 " << expectedValue_.e0.w << ", " << expectedValue_.e1.w << "]";
    }
private:
    const odb::Mat4x2<float> expectedValue_;
};

template <>
class LiteralEqMatcher<odb::Mat4x3<float>> : public MatcherInterface<const ast::LiteralTemplate<odb::Mat4x3<float>>*>
{
public:
    explicit LiteralEqMatcher(const odb::Mat4x3<float>& expectedValue)
        : expectedValue_(expectedValue) {}
    bool MatchAndExplain(const ast::LiteralTemplate<odb::Mat4x3<float>>* literal, MatchResultListener* listener) const override {
        *listener << "literal->value() equals [" << literal->value().e0.x << ", " << literal->value().e1.x << ";\n"
                  << "                         " << literal->value().e0.y << ", " << literal->value().e1.y << ";\n"
                  << "                         " << literal->value().e0.y << ", " << literal->value().e1.y << ";\n"
                  << "                         " << literal->value().e0.y << ", " << literal->value().e1.y << "]";
        return literal->value().e0.x == expectedValue_.e0.x && literal->value().e1.x == expectedValue_.e1.x && literal->value().e2.x == expectedValue_.e2.x
            && literal->value().e0.y == expectedValue_.e0.y && literal->value().e1.y == expectedValue_.e1.y && literal->value().e2.y == expectedValue_.e2.y
            && literal->value().e0.z == expectedValue_.e0.z && literal->value().e1.z == expectedValue_.e1.z && literal->value().e2.z == expectedValue_.e2.z
            && literal->value().e0.w == expectedValue_.e0.w && literal->value().e1.w == expectedValue_.e1.w && literal->value().e2.w == expectedValue_.e2.w;
    }
    void DescribeTo(::std::ostream* os) const override {
        *os << "literal->value() equals [" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ", " << expectedValue_.e2.x << ";\n"
            << "                         " << expectedValue_.e0.y << ", " << expectedValue_.e1.y << ", " << expectedValue_.e2.y << ";\n"
            << "                         " << expectedValue_.e0.z << ", " << expectedValue_.e1.z << ", " << expectedValue_.e2.z << ";\n"
            << "                         " << expectedValue_.e0.w << ", " << expectedValue_.e1.w << ", " << expectedValue_.e2.w << "]";
    }
    void DescribeNegationTo(::std::ostream* os) const override {
        *os << "literal->value() does not equal [" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ", " << expectedValue_.e2.x << ";\n"
            << "                                 " << expectedValue_.e0.y << ", " << expectedValue_.e1.y << ", " << expectedValue_.e2.y << ";\n"
            << "                                 " << expectedValue_.e0.z << ", " << expectedValue_.e1.z << ", " << expectedValue_.e2.z << ";\n"
            << "                                 " << expectedValue_.e0.w << ", " << expectedValue_.e1.w << ", " << expectedValue_.e2.w << "]";
    }
private:
    const odb::Mat4x3<float> expectedValue_;
};

template <>
class LiteralEqMatcher<odb::Mat4x4<float>> : public MatcherInterface<const ast::LiteralTemplate<odb::Mat4x4<float>>*>
{
public:
    explicit LiteralEqMatcher(const odb::Mat4x4<float>& expectedValue)
        : expectedValue_(expectedValue) {}
    bool MatchAndExplain(const ast::LiteralTemplate<odb::Mat4x4<float>>* literal, MatchResultListener* listener) const override {
        *listener << "literal->value() equals [" << literal->value().e0.x << ", " << literal->value().e1.x << ", " << literal->value().e2.x << ", " << literal->value().e3.x << ";\n"
                  << "                         " << literal->value().e0.y << ", " << literal->value().e1.y << ", " << literal->value().e2.y << ", " << literal->value().e3.y << ";\n"
                  << "                         " << literal->value().e0.z << ", " << literal->value().e1.z << ", " << literal->value().e2.z << ", " << literal->value().e3.z << ";\n"
                  << "                         " << literal->value().e0.w << ", " << literal->value().e1.w << ", " << literal->value().e2.w << ", " << literal->value().e3.w << "]";
        return literal->value().e0.x == expectedValue_.e0.x && literal->value().e1.x == expectedValue_.e1.x && literal->value().e2.x == expectedValue_.e2.x && literal->value().e3.x == expectedValue_.e3.x
            && literal->value().e0.y == expectedValue_.e0.y && literal->value().e1.y == expectedValue_.e1.y && literal->value().e2.y == expectedValue_.e2.y && literal->value().e3.y == expectedValue_.e3.y
            && literal->value().e0.z == expectedValue_.e0.z && literal->value().e1.z == expectedValue_.e1.z && literal->value().e2.z == expectedValue_.e2.z && literal->value().e3.z == expectedValue_.e3.z
            && literal->value().e0.w == expectedValue_.e0.w && literal->value().e1.w == expectedValue_.e1.w && literal->value().e2.w == expectedValue_.e2.w && literal->value().e3.w == expectedValue_.e3.w;
    }
    void DescribeTo(::std::ostream* os) const override {
        *os << "literal->value() equals [" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ", " << expectedValue_.e2.x << ", " << expectedValue_.e3.x << ";\n"
            << "                         " << expectedValue_.e0.y << ", " << expectedValue_.e1.y << ", " << expectedValue_.e2.y << ", " << expectedValue_.e3.y << ";\n"
            << "                         " << expectedValue_.e0.z << ", " << expectedValue_.e1.z << ", " << expectedValue_.e2.z << ", " << expectedValue_.e3.z << ";\n"
            << "                         " << expectedValue_.e0.w << ", " << expectedValue_.e1.w << ", " << expectedValue_.e2.w << ", " << expectedValue_.e3.w << "]";
    }
    void DescribeNegationTo(::std::ostream* os) const override {
        *os << "literal->value() does not equal [" << expectedValue_.e0.x << ", " << expectedValue_.e1.x << ", " << expectedValue_.e2.x << ", " << expectedValue_.e3.x << ";\n"
            << "                                 " << expectedValue_.e0.y << ", " << expectedValue_.e1.y << ", " << expectedValue_.e2.y << ", " << expectedValue_.e3.y << ";\n"
            << "                                 " << expectedValue_.e0.z << ", " << expectedValue_.e1.z << ", " << expectedValue_.e2.z << ", " << expectedValue_.e3.z << ";\n"
            << "                                 " << expectedValue_.e0.w << ", " << expectedValue_.e1.w << ", " << expectedValue_.e2.w << ", " << expectedValue_.e3.w << "]";
    }
private:
    const odb::Mat4x4<float> expectedValue_;
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
