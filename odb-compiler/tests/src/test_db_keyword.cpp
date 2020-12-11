#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"

#define NAME db_keyword

using namespace testing;
using namespace odb;

class NAME : public ParserTestHarness
{
public:
};

TEST_F(NAME, print_command)
{
    kwIndex.addKeyword(new Keyword(nullptr, "print", "", Keyword::Type::Void, {}));
    matcher.updateFromIndex(&kwIndex);
    ast = driver->parseString("test",
        "print \"hello world\"\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitKeywordStmntSymbol(KeywordStmntSymbolEq("print"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitStringLiteral(StringLiteralEq("hello world"))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, command_with_spaces)
{
    kwIndex.addKeyword(new Keyword(nullptr, "make object sphere", "", Keyword::Type::Void, {}));
    matcher.updateFromIndex(&kwIndex);
    ast = driver->parseString("test",
        "make object sphere 1, 10\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitKeywordStmntSymbol(KeywordStmntSymbolEq("make object sphere"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitBooleanLiteral(BooleanLiteralEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(10))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, randomize_timer)
{
    kwIndex.addKeyword(new Keyword(nullptr, "randomize", "", Keyword::Type::Void, {}));
    kwIndex.addKeyword(new Keyword(nullptr, "timer", "", Keyword::Type::Void, {}));
    matcher.updateFromIndex(&kwIndex);
    ast = driver->parseString("test",
        "randomize timer()\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitKeywordStmntSymbol(KeywordStmntSymbolEq("randomize"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitKeywordExprSymbol(KeywordExprSymbolEq("timer"))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, randomize_timer_args)
{
    kwIndex.addKeyword(new Keyword(nullptr, "randomize", "", Keyword::Type::Void, {}));
    kwIndex.addKeyword(new Keyword(nullptr, "timer", "", Keyword::Type::Void, {}));
    matcher.updateFromIndex(&kwIndex);
    ast = driver->parseString("test",
        "randomize timer(5)\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitKeywordStmntSymbol(KeywordStmntSymbolEq("randomize"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitKeywordExprSymbol(KeywordExprSymbolEq("timer"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(5))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, keyword_with_string_annotation)
{
    kwIndex.addKeyword(new Keyword(nullptr, "str$", "", Keyword::Type::Void, {}));
    kwIndex.addKeyword(new Keyword(nullptr, "print", "", Keyword::Type::Void, {}));
    matcher.updateFromIndex(&kwIndex);
    ast = driver->parseString("test",
        "print str$(5)\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitKeywordStmntSymbol(KeywordStmntSymbolEq("print"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitKeywordExprSymbol(KeywordExprSymbolEq("str$"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(5))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, keyword_with_float_annotation)
{
    kwIndex.addKeyword(new Keyword(nullptr, "str#", "", Keyword::Type::Void, {}));
    kwIndex.addKeyword(new Keyword(nullptr, "print", "", Keyword::Type::Void, {}));
    matcher.updateFromIndex(&kwIndex);
    ast = driver->parseString("test",
        "print str#(5)\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitKeywordStmntSymbol(KeywordStmntSymbolEq("print"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitKeywordExprSymbol(KeywordExprSymbolEq("str#"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(5))).After(exp);

    ast->accept(&v);
}

/* TODO: Implement VarRef first
TEST_F(NAME, load_3d_sound)
{
    using Annotation = ast::Symbol::Annotation;

    kwIndex.addKeyword(new Keyword(nullptr, "load 3dsound", "", Keyword::Type::Void, {}));
    matcher.updateFromIndex(&kwIndex);
    ast = driver->parseString("test",
        "load 3dsound \"howl.wav\",s\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitKeywordStmntSymbol(_)).After(exp);
    exp = EXPECT_CALL(v, visitSymbol(SymbolEq("load 3dsound"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitStringLiteral(StringLiteralEq("howl.wav"))).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq("s", Annotation::NONE))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, command_with_variable_args)
{
    kwIndex.addKeyword({"clone sound", "", "", {}, std::nullopt});
    matcher.updateFromIndex(&kwIndex);
    ASSERT_THAT(driver->parseString(
        "clone sound s,2\n"), IsTrue());
}

TEST_F(NAME, command_with_spaces_as_argument_to_command_with_spaces)
{
    kwIndex.addKeyword({"make object sphere", "", "", {}, std::nullopt});
    kwIndex.addKeyword({"get ground height", "", "", {}, {Keyword::Type::Integer}});
    matcher.updateFromIndex(&kwIndex);
    ASSERT_THAT(driver->parseString(
        "make object sphere get ground height(1, x, y), 10\n"), IsTrue());
}*/

TEST_F(NAME, keyword_starting_with_builtin)
{
    // "loop" is a builtin keyword
    kwIndex.addKeyword(new Keyword(nullptr, "loop", "", Keyword::Type::Void, {}));
    kwIndex.addKeyword(new Keyword(nullptr, "loop sound", "", Keyword::Type::Void, {}));
    matcher.updateFromIndex(&kwIndex);
    ast = driver->parseString("test",
        "loop sound 1\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitKeywordStmntSymbol(KeywordStmntSymbolEq("loop sound"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitBooleanLiteral(BooleanLiteralEq(1))).After(exp);

    ast->accept(&v);
}

/*
TEST_F(NAME, builtin_shadowing_keyword)
{
    // "loop" is a builtin keyword
    kwIndex.addKeyword({"loop", "", "", {}, std::nullopt});
    kwIndex.addKeyword({"loop sound", "", "", {}, std::nullopt});
    matcher.updateFromIndex(&kwIndex);
    ASSERT_THAT(driver->parseString("do : foo() : loop"), IsTrue());
}

TEST_F(NAME, multiple_similar_keywords_with_spaces)
{
    // "loop" is a builtin keyword
    kwIndex.addKeyword({"set object", "", "", {}, std::nullopt});
    kwIndex.addKeyword({"set object speed", "", "", {}, std::nullopt});
    matcher.updateFromIndex(&kwIndex);
    ASSERT_THAT(driver->parseString("set object speed 1, 10\n"), IsTrue());
}

TEST_F(NAME, multiple_similar_keywords_with_spaces_2)
{
    kwIndex.addKeyword({"SET OBJECT AMBIENT", "", "", {}});
    kwIndex.addKeyword({"SET OBJECT COLLISION ON", "", "", {}});
    kwIndex.addKeyword({"SET OBJECT COLLISION OFF", "", "", {}});
    kwIndex.addKeyword({"SET OBJECT COLLISION TO BOXES", "", "", {}});
    kwIndex.addKeyword({"SET OBJECT", "", "", {}});
    kwIndex.addKeyword({"set object collision off", "", "", {}});
    matcher.updateFromIndex(&kwIndex);
    ASSERT_THAT(driver->parseString("set object collision off 1\n"), IsTrue());
}

TEST_F(NAME, incomplete_keyword_at_end_of_file)
{
    kwIndex.addKeyword({"color object", "", "", {}});
    matcher.updateFromIndex(&kwIndex);
    ASSERT_THAT(driver->parseString(
        "function foo()\n"
        "    a = 2\n"
        "endfunction color"), IsTrue());
}

TEST_F(NAME, keywords_with_type)
{
    kwIndex.addKeyword({"get dir$", "", "", {}, {Keyword::Type::Integer}});
    matcher.updateFromIndex(&kwIndex);
    ASSERT_THAT(driver->parseString(
        "OriginalDirectory$ = get dir$()"), IsTrue());
}

TEST_F(NAME, keyword_containing_builtin_in_middle)
{
    kwIndex.addKeyword({"set effect constant boolean", "", "", {}});
    kwIndex.addKeyword({"set effect constant float", "", "", {}});
    matcher.updateFromIndex(&kwIndex);
    ASSERT_THAT(driver->parseString(
        "set effect constant float RingsFX, \"shrink\", BlackHoleFunnel(0).shrink#\n"), IsTrue());
}

TEST_F(NAME, keyword_variable_name)
{
    kwIndex.addKeyword({"text", "", "", {}});
    matcher.updateFromIndex(&kwIndex);
    ASSERT_THAT(driver->parseString(
        "text$ as string"), IsTrue());
}

TEST_F(NAME, builtin_keyword_variable_name)
{
    matcher.updateFromIndex(&kwIndex);
    ASSERT_THAT(driver->parseString(
        "string$ as string"), IsTrue());
}
*/
