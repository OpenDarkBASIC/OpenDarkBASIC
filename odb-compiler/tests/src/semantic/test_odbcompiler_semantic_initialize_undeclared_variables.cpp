#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-sdk/tests/LogHelper.hpp"

#include <gmock/gmock.h>

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_initialize_undeclared_variables

using namespace testing;

struct Utf8SpanEqMatcher : testing::MatcherInterface<const struct utf8_span&>
{
    explicit Utf8SpanEqMatcher(utf8_idx off, utf8_idx len)
        : expected({off, len})
    {
    }

    bool
    MatchAndExplain(
        const struct utf8_span&       span,
        testing::MatchResultListener* listener) const override
    {
        *listener << "{" << span.off << ", " << span.len << "}";
        return span.off == expected.off && span.len == expected.len;
    }

    void
    DescribeTo(::std::ostream* os) const override
    {
        *os << "Span equals {" << expected.off << ", " << expected.len << "}";
    }
    void
    DescribeNegationTo(::std::ostream* os) const override
    {
        *os << "Span does not equal {" << expected.off << ", " << expected.len
            << "}";
    }

    struct utf8_span expected;
};

inline testing::Matcher<const struct utf8_span&>
Utf8SpanEq(utf8_idx off, utf8_idx len)
{
    return testing::MakeMatcher(new Utf8SpanEqMatcher(off, len));
}

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, integer_initialized_to_0)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_INTEGER});
    const char* source = "print a";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check_and_cast, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ast_id ass = ast.nodes[0].block.stmt;
    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(6, 1));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[init].integer_literal.value, Eq(0));
}

TEST_F(NAME, double_integer_initialized_to_0)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_DOUBLE_INTEGER});
    const char* source = "print a&";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check_and_cast, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ast_id ass = ast.nodes[0].block.stmt;
    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(6, 2));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_DOUBLE_INTEGER_LITERAL));
    EXPECT_THAT(ast.nodes[init].integer_literal.value, Eq(0));
}

TEST_F(NAME, word_initialized_to_0)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_WORD});
    const char* source = "print a%";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check_and_cast, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ast_id ass = ast.nodes[0].block.stmt;
    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(6, 2));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_WORD_LITERAL));
    EXPECT_THAT(ast.nodes[init].integer_literal.value, Eq(0));
}

TEST_F(NAME, double_initialized_to_0)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_DOUBLE});
    const char* source = "print a!";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check_and_cast, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ast_id ass = ast.nodes[0].block.stmt;
    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(6, 2));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_DOUBLE_LITERAL));
    EXPECT_THAT(ast.nodes[init].integer_literal.value, Eq(0));
}

TEST_F(NAME, float_initialized_to_0)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_FLOAT});
    const char* source = "print a#";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check_and_cast, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ast_id ass = ast.nodes[0].block.stmt;
    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(6, 2));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_FLOAT_LITERAL));
    EXPECT_THAT(ast.nodes[init].integer_literal.value, Eq(0));
}

TEST_F(NAME, string_initialized_to_0)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_STRING});
    const char* source = "print a$";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(
        semantic_check_run(
            &semantic_type_check_and_cast, &ast, plugins, &cmds, "test", src),
        Eq(0))
        << log().text;
    ast_id ass = ast.nodes[0].block.stmt;
    ast_id var = ast.nodes[ass].assignment.lvalue;
    ast_id init = ast.nodes[ass].assignment.expr;
    EXPECT_THAT(ast.nodes[var].info.node_type, Eq(AST_IDENTIFIER));
    EXPECT_THAT(ast.nodes[var].identifier.name, Utf8SpanEq(6, 2));
    EXPECT_THAT(ast.nodes[init].info.node_type, Eq(AST_STRING_LITERAL));
    EXPECT_THAT(ast.nodes[init].integer_literal.value, Eq(0));
}
