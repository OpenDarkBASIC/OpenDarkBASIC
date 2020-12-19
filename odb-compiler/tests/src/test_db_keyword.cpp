#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/ast/Keyword.hpp"
#include "odb-compiler/keywords/Keyword.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"

#define NAME db_command

using namespace testing;
using namespace odb;

class NAME : public ParserTestHarness
{
public:
};

TEST_F(NAME, print_command)
{
    kwIndex.addKeyword(new kw::Keyword(nullptr, "print", "", kw::Keyword::Type::Void, {}));
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
    kwIndex.addKeyword(new kw::Keyword(nullptr, "make object sphere", "", kw::Keyword::Type::Void, {}));
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
    kwIndex.addKeyword(new kw::Keyword(nullptr, "randomize", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "timer", "", kw::Keyword::Type::Void, {}));
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
    kwIndex.addKeyword(new kw::Keyword(nullptr, "randomize", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "timer", "", kw::Keyword::Type::Void, {}));
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

TEST_F(NAME, command_with_string_annotation)
{
    kwIndex.addKeyword(new kw::Keyword(nullptr, "str$", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "print", "", kw::Keyword::Type::Void, {}));
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

TEST_F(NAME, command_with_float_annotation)
{
    kwIndex.addKeyword(new kw::Keyword(nullptr, "str#", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "print", "", kw::Keyword::Type::Void, {}));
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

TEST_F(NAME, load_3d_sound)
{
    using Annotation = ast::Symbol::Annotation;

    kwIndex.addKeyword(new kw::Keyword(nullptr, "load 3dsound", "", kw::Keyword::Type::Void, {}));
    matcher.updateFromIndex(&kwIndex);
    ast = driver->parseString("test",
        "load 3dsound \"howl.wav\",s\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitKeywordStmntSymbol(_)).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitStringLiteral(StringLiteralEq("howl.wav"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "s"))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, command_with_variable_args)
{
    using Annotation = ast::Symbol::Annotation;

    kwIndex.addKeyword(new kw::Keyword(nullptr, "clone sound", "", kw::Keyword::Type::Void, {}));
    matcher.updateFromIndex(&kwIndex);
    ast = driver->parseString("test",
        "clone sound s,2\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitKeywordStmntSymbol(KeywordStmntSymbolEq("clone sound"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "s"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, command_with_spaces_as_argument_to_command_with_spaces)
{
    using Annotation = ast::Symbol::Annotation;

    kwIndex.addKeyword(new kw::Keyword(nullptr, "make object sphere", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "get ground height", "", kw::Keyword::Type::Void, {}));
    matcher.updateFromIndex(&kwIndex);
    ast = driver->parseString("test",
        "make object sphere get ground height(2, x, y), 10\n");
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitKeywordStmntSymbol(KeywordStmntSymbolEq("make object sphere"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitKeywordExprSymbol(KeywordExprSymbolEq("get ground height"))).After(exp);
    exp = EXPECT_CALL(v, visitExpressionList(ExpressionListCountEq(3))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "x"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "y"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(10))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, command_starting_with_builtin)
{
    // "loop" is a builtin command
    kwIndex.addKeyword(new kw::Keyword(nullptr, "loop", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "loop sound", "", kw::Keyword::Type::Void, {}));
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

TEST_F(NAME, builtin_shadowing_command)
{
    // "loop" is a builtin command
    kwIndex.addKeyword(new kw::Keyword(nullptr, "loop", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "loop sound", "", kw::Keyword::Type::Void, {}));
    matcher.updateFromIndex(&kwIndex);
    ast = driver->parseString("test",
        "do : foo() : loop");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, multiple_similar_commands_with_spaces)
{
    kwIndex.addKeyword(new kw::Keyword(nullptr, "set object", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "set object speed", "", kw::Keyword::Type::Void, {}));
    matcher.updateFromIndex(&kwIndex);
    ast = driver->parseString("test",
        "set object speed 1, 10\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, multiple_similar_commands_with_spaces_2)
{
    kwIndex.addKeyword(new kw::Keyword(nullptr, "SET OBJECT AMBIENT", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "SET OBJECT COLLISION ON", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "SET OBJECT COLLISION OFF", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "SET OBJECT COLLISION ON", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "SET OBJECT", "", kw::Keyword::Type::Void, {}));
    matcher.updateFromIndex(&kwIndex);
    ast = driver->parseString("test",
        "set object collision off 1\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, incomplete_command_at_end_of_file)
{
    kwIndex.addKeyword(new kw::Keyword(nullptr, "color object", "", kw::Keyword::Type::Void, {}));
    matcher.updateFromIndex(&kwIndex);
    ast = driver->parseString("test",
        "function foo()\n"
        "    a = 2\n"
        "endfunction color");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, commands_with_type)
{
    kwIndex.addKeyword(new kw::Keyword(nullptr, "get dir$", "", kw::Keyword::Type::Void, {}));
    matcher.updateFromIndex(&kwIndex);
    ast = driver->parseString("test",
        "OriginalDirectory$ = get dir$()");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, command_containing_builtin_in_middle)
{
    kwIndex.addKeyword(new kw::Keyword(nullptr, "set effect constant boolean", "", kw::Keyword::Type::Void, {}));
    kwIndex.addKeyword(new kw::Keyword(nullptr, "set effect constant float", "", kw::Keyword::Type::Void, {}));
    matcher.updateFromIndex(&kwIndex);
    ast = driver->parseString("test",
        "set effect constant float RingsFX, \"shrink\", BlackHoleFunnel(0).shrink#\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, command_variable_name)
{
    kwIndex.addKeyword(new kw::Keyword(nullptr, "text", "", kw::Keyword::Type::Void, {}));
    matcher.updateFromIndex(&kwIndex);
    ast = driver->parseString("test",
        "text$ as string");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, builtin_keyword_variable_name_1)
{
    matcher.updateFromIndex(&kwIndex);
    ast = driver->parseString("test",
        "string$ as string");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, builtin_keyword_variable_name_2)
{
    matcher.updateFromIndex(&kwIndex);
    ast = driver->parseString("test",
        "string# as float");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, command_variable_name_1)
{
    kwIndex.addKeyword(new kw::Keyword(nullptr, "command", "", kw::Keyword::Type::Void, {}));
    matcher.updateFromIndex(&kwIndex);
    ast = driver->parseString("test",
        "command$ as string");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, command_variable_name_2)
{
    kwIndex.addKeyword(new kw::Keyword(nullptr, "command", "", kw::Keyword::Type::Void, {}));
    matcher.updateFromIndex(&kwIndex);
    ast = driver->parseString("test",
        "command# as float");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, command_with_same_name_as_keyword)
{
    kwIndex.addKeyword(new kw::Keyword(nullptr, "loop", "", kw::Keyword::Type::Void, {}));
    matcher.updateFromIndex(&kwIndex);
    ast = driver->parseString("test",
        "do\nloop");
    ASSERT_THAT(ast, NotNull());
}
