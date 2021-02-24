#include "odb-compiler/ast/Annotation.hpp"
#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/commands/Command.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/matchers/AnnotatedSymbolEq.hpp"
#include "odb-compiler/tests/matchers/ArgListCountEq.hpp"
#include "odb-compiler/tests/matchers/BlockStmntCountEq.hpp"
#include "odb-compiler/tests/matchers/CommandExprEq.hpp"
#include "odb-compiler/tests/matchers/CommandStmntEq.hpp"
#include "odb-compiler/tests/matchers/LiteralEq.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"

#define NAME db_parser_command

using namespace testing;
using namespace odb;
using namespace ast;

class NAME : public ParserTestHarness
{
public:
};

#define X(enum_, chr, str, dbname)                                            \
TEST_F(NAME, command_expr_with_type_annotation_##enum_)                       \
{                                                                             \
    cmdIndex.addCommand(new cmd::Command(nullptr, "get dir" str, "", cmd::Command::Type::Void, {}));\
    matcher.updateFromIndex(&cmdIndex);                                       \
    ast = driver->parse("test",                                               \
        "OriginalDirectory" str " = get dir" str "()",                        \
        matcher);                                                             \
    ASSERT_THAT(ast, NotNull());                                              \
}
ODB_TYPE_ANNOTATION_LIST
#undef X

TEST_F(NAME, print_command)
{
    cmdIndex.addCommand(new cmd::Command(nullptr, "print", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test",
        "print \"hello world\"\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitCommandStmnt(CommandStmntEq("print"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitStringLiteral(StringLiteralEq("hello world"))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, command_with_spaces)
{
    cmdIndex.addCommand(new cmd::Command(nullptr, "make object sphere", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test",
        "make object sphere 1, 10\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitCommandStmnt(CommandStmntEq("make object sphere"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(10))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, randomize_timer)
{
    cmdIndex.addCommand(new cmd::Command(nullptr, "randomize", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(nullptr, "timer", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test",
        "randomize timer()\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitCommandStmnt(CommandStmntEq("randomize"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitCommandExpr(CommandExprEq("timer"))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, randomize_timer_args)
{
    cmdIndex.addCommand(new cmd::Command(nullptr, "randomize", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(nullptr, "timer", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test",
        "randomize timer(5)\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitCommandStmnt(CommandStmntEq("randomize"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitCommandExpr(CommandExprEq("timer"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(5))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, command_with_string_annotation)
{
    cmdIndex.addCommand(new cmd::Command(nullptr, "str$", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(nullptr, "print", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test",
        "print str$(5)\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitCommandStmnt(CommandStmntEq("print"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitCommandExpr(CommandExprEq("str$"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(5))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, command_with_float_annotation)
{
    cmdIndex.addCommand(new cmd::Command(nullptr, "str#", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(nullptr, "print", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test",
        "print str#(5)\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitCommandStmnt(CommandStmntEq("print"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitCommandExpr(CommandExprEq("str#"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(5))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, load_3d_sound)
{
    cmdIndex.addCommand(new cmd::Command(nullptr, "load 3dsound", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test",
        "load 3dsound \"howl.wav\",s\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitCommandStmnt(_)).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitStringLiteral(StringLiteralEq("howl.wav"))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "s"))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, command_with_variable_args)
{
    cmdIndex.addCommand(new cmd::Command(nullptr, "clone sound", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test",
        "clone sound s,2\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitCommandStmnt(CommandStmntEq("clone sound"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "s"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, command_with_spaces_as_argument_to_command_with_spaces)
{
    cmdIndex.addCommand(new cmd::Command(nullptr, "make object sphere", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(nullptr, "get ground height", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test",
        "make object sphere get ground height(2, x, y), 10\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitCommandStmnt(CommandStmntEq("make object sphere"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitCommandExpr(CommandExprEq("get ground height"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(3))).After(exp);
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
    cmdIndex.addCommand(new cmd::Command(nullptr, "loop", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(nullptr, "loop sound", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test",
        "loop sound 1\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitCommandStmnt(CommandStmntEq("loop sound"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(1))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, builtin_shadowing_command)
{
    // "loop" is a builtin command
    cmdIndex.addCommand(new cmd::Command(nullptr, "loop", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(nullptr, "loop sound", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test",
        "do : foo() : loop",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitInfiniteLoop(_)).After(exp);
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitFuncCallStmnt(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "foo"))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, multiple_similar_commands_with_spaces)
{
    cmdIndex.addCommand(new cmd::Command(nullptr, "set object", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(nullptr, "set object speed", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test",
        "set object speed 1, 10\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitCommandStmnt(CommandStmntEq("set object speed"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(10))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, multiple_similar_commands_with_spaces_2)
{
    cmdIndex.addCommand(new cmd::Command(nullptr, "SET OBJECT AMBIENT", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(nullptr, "SET OBJECT COLLISION ON", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(nullptr, "SET OBJECT COLLISION OFF", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(nullptr, "SET OBJECT COLLISION ON", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(nullptr, "SET OBJECT", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test",
        "set object collision off 1\n",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitCommandStmnt(CommandStmntEq("set object collision off"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(1))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, incomplete_command_at_end_of_file)
{
    cmdIndex.addCommand(new cmd::Command(nullptr, "color object", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test",
        "function foo()\n"
        "    a = 2\n"
        "endfunction color",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation exp;
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
    exp = EXPECT_CALL(v, visitFuncDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "foo"))).After(exp);
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "a"))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "color"))).After(exp);

    ast->accept(&v);
}

TEST_F(NAME, command_containing_builtin_in_middle)
{
    cmdIndex.addCommand(new cmd::Command(nullptr, "set effect constant boolean", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(nullptr, "set effect constant float", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test",
        "set effect constant float RingsFX, \"shrink\", BlackHoleFunnel(0).shrink#\n",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, command_variable_name)
{
    cmdIndex.addCommand(new cmd::Command(nullptr, "text", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test",
        "text$ as string",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, builtin_keyword_variable_name_1)
{
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test",
        "string$ as string",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, builtin_keyword_variable_name_2)
{
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test",
        "string# as float",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, command_variable_name_1)
{
    cmdIndex.addCommand(new cmd::Command(nullptr, "command", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test",
        "command$ as string",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, command_variable_name_2)
{
    cmdIndex.addCommand(new cmd::Command(nullptr, "command", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test",
        "command# as float",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, command_with_same_name_as_keyword)
{
    cmdIndex.addCommand(new cmd::Command(nullptr, "loop", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test",
        "do\nloop",
        matcher);
    ASSERT_THAT(ast, NotNull());
}
