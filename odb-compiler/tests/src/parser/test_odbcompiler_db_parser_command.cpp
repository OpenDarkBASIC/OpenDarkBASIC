#include "odb-compiler/tests/DBParserHelper.hpp"

#include <gmock/gmock.h>

extern "C" {
#include "odb-compiler/ast/ast.h"
}

#define NAME odbcompiler_db_parser_command

using namespace testing;

struct NAME : DBParserHelper, Test
{
};

TEST_F(NAME, print_command)
{
    addCommand("PRINP");
    addCommand("PRINT");
    addCommand("PRINT STDOUT");
    ASSERT_THAT(parse("print \"hello world\"\n"), Eq(0));

    ASSERT_THAT(ast_count(ast), Eq(4));
    ASSERT_THAT(ast_node_type(ast, ast->root), Eq(AST_BLOCK));
    ASSERT_THAT(ast->nodes[ast->root].block.next, Eq(-1));
    ast_id stmt = ast->nodes[ast->root].block.stmt;
    ASSERT_THAT(stmt, Gt(-1));
    ASSERT_THAT(ast_node_type(ast, stmt), Eq(AST_COMMAND));
    ASSERT_THAT(ast->nodes[stmt].cmd.id, Eq(1));
    ast_id arglist = ast->nodes[stmt].cmd.arglist;
    ASSERT_THAT(arglist, Gt(-1));
    ASSERT_THAT(ast_node_type(ast, arglist), Eq(AST_ARGLIST));
    ASSERT_THAT(ast->nodes[arglist].arglist.next, Eq(-1));
    ast_id expr = ast->nodes[arglist].arglist.expr;
    ASSERT_THAT(expr, Gt(-1));
    ASSERT_THAT(ast_node_type(ast, expr), Eq(AST_STRING_LITERAL));
    ASSERT_THAT(ast->nodes[expr].string_literal.str.off, Eq(7));
    ASSERT_THAT(ast->nodes[expr].string_literal.str.len, Eq(11));
}

TEST_F(NAME, command_expr_with_type_annotation_int64)
{
    addCommand(TYPE_I64, "GET DIR&");

    ASSERT_THAT(parse("OriginalDirectory& = get dir&()"), Eq(0));
}

TEST_F(NAME, command_with_spaces)
{
    addCommand("MAKE OBJECT SPHERE");
    ASSERT_THAT(parse("make object sphere 1, 10\n"), Eq(0));

    /*
    StrictMock<ASTMockVisitor> v;
    Expectation                exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitCommandStmnt(CommandStmntEq("make object
sphere"))).After(exp); exp = EXPECT_CALL(v,
visitArgList(ArgListCountEq(2))).After(exp); exp = EXPECT_CALL(v,
visitByteLiteral(ByteLiteralEq(1))).After(exp); exp = EXPECT_CALL(v,
visitByteLiteral(ByteLiteralEq(10))).After(exp);

    visitAST(ast, v);*/
}

/*
TEST_F(NAME, randomize_timer)
{
    cmdIndex.addCommand(new cmd::Command(
        nullptr, "randomize", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(
        new cmd::Command(nullptr, "timer", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test", "randomize timer()\n", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation                exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitCommandStmnt(CommandStmntEq("randomize")))
              .After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitCommandExpr(CommandExprEq("timer"))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, randomize_timer_args)
{
    cmdIndex.addCommand(new cmd::Command(
        nullptr, "randomize", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(
        new cmd::Command(nullptr, "timer", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test", "randomize timer(5)\n", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation                exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitCommandStmnt(CommandStmntEq("randomize")))
              .After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitCommandExpr(CommandExprEq("timer"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(5))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, command_with_string_annotation)
{
    cmdIndex.addCommand(
        new cmd::Command(nullptr, "str$", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(
        new cmd::Command(nullptr, "print", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test", "print str$(5)\n", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation                exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitCommandStmnt(CommandStmntEq("print"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitCommandExpr(CommandExprEq("str$"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(5))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, command_with_float_annotation)
{
    cmdIndex.addCommand(
        new cmd::Command(nullptr, "str#", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(
        new cmd::Command(nullptr, "print", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test", "print str#(5)\n", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation                exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitCommandStmnt(CommandStmntEq("print"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitCommandExpr(CommandExprEq("str#"))).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(5))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, load_3d_sound)
{
    cmdIndex.addCommand(new cmd::Command(
        nullptr, "load 3dsound", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test", "load 3dsound \"howl.wav\",s\n", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation                exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitCommandStmnt(_)).After(exp);
    exp = EXPECT_CALL(v, visitArgList(ArgListCountEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitStringLiteral(StringLiteralEq("howl.wav")))
              .After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("s", Annotation::NONE)))
              .After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, command_with_variable_args)
{
    cmdIndex.addCommand(new cmd::Command(
        nullptr, "clone sound", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test", "clone sound s,2\n", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation                exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitCommandStmnt(CommandStmntEq("clone
sound"))).After(exp); exp = EXPECT_CALL(v,
visitArgList(ArgListCountEq(2))).After(exp); exp = EXPECT_CALL(v,
visitVarRef(_)).After(exp); exp = EXPECT_CALL(v,
visitIdentifier(IdentifierEq("s", Annotation::NONE))).After(exp); exp =
EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, command_with_spaces_as_argument_to_command_with_spaces)
{
    cmdIndex.addCommand(new cmd::Command(
        nullptr, "make object sphere", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(
        nullptr, "get ground height", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse(
        "test", "make object sphere get ground height(2, x, y), 10\n", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation                exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitCommandStmnt(CommandStmntEq("make object
sphere"))).After(exp); exp = EXPECT_CALL(v,
visitArgList(ArgListCountEq(2))).After(exp); exp = EXPECT_CALL(v,
visitCommandExpr(CommandExprEq("get ground height"))).After(exp); exp =
EXPECT_CALL(v, visitArgList(ArgListCountEq(3))).After(exp); exp = EXPECT_CALL(v,
visitByteLiteral(ByteLiteralEq(2))).After(exp); exp = EXPECT_CALL(v,
visitVarRef(_)).After(exp); exp = EXPECT_CALL(v,
visitIdentifier(IdentifierEq("x", Annotation::NONE))).After(exp); exp =
EXPECT_CALL(v, visitVarRef(_)).After(exp); exp = EXPECT_CALL(v,
visitIdentifier(IdentifierEq("y", Annotation::NONE))).After(exp); exp =
EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(10))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, command_starting_with_builtin)
{
    // "loop" is a builtin command
    cmdIndex.addCommand(
        new cmd::Command(nullptr, "loop", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(
        nullptr, "loop sound", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test", "loop sound 1\n", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation                exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitCommandStmnt(CommandStmntEq("loop
sound"))).After(exp); exp = EXPECT_CALL(v,
visitArgList(ArgListCountEq(1))).After(exp); exp = EXPECT_CALL(v,
visitByteLiteral(ByteLiteralEq(1))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, builtin_shadowing_command)
{
    // "loop" is a builtin command
    cmdIndex.addCommand(
        new cmd::Command(nullptr, "loop", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(
        nullptr, "loop sound", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test", "do : foo() : loop", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation                exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitInfiniteLoop(_)).After(exp);
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitFuncCallStmnt(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("foo", Annotation::NONE)))
              .After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, multiple_similar_commands_with_spaces)
{
    cmdIndex.addCommand(new cmd::Command(
        nullptr, "set object", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(
        nullptr, "set object speed", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test", "set object speed 1, 10\n", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation                exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitCommandStmnt(CommandStmntEq("set object
speed"))).After(exp); exp = EXPECT_CALL(v,
visitArgList(ArgListCountEq(2))).After(exp); exp = EXPECT_CALL(v,
visitByteLiteral(ByteLiteralEq(1))).After(exp); exp = EXPECT_CALL(v,
visitByteLiteral(ByteLiteralEq(10))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, multiple_similar_commands_with_spaces_2)
{
    cmdIndex.addCommand(new cmd::Command(
        nullptr, "SET OBJECT AMBIENT", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(
        nullptr, "SET OBJECT COLLISION ON", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(
        nullptr, "SET OBJECT COLLISION OFF", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(
        nullptr, "SET OBJECT COLLISION ON", "", cmd::Command::Type::Void, {}));
    cmdIndex.addCommand(new cmd::Command(
        nullptr, "SET OBJECT", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test", "set object collision off 1\n", matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation                exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitCommandStmnt(CommandStmntEq("set object collision
off"))).After(exp); exp = EXPECT_CALL(v,
visitArgList(ArgListCountEq(1))).After(exp); exp = EXPECT_CALL(v,
visitByteLiteral(ByteLiteralEq(1))).After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, incomplete_command_at_end_of_file)
{
    cmdIndex.addCommand(new cmd::Command(
        nullptr, "color object", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse(
        "test",
        "function foo()\n"
        "    a = 2\n"
        "endfunction color",
        matcher);
    ASSERT_THAT(ast, NotNull());

    StrictMock<ASTMockVisitor> v;
    Expectation                exp;
    exp = EXPECT_CALL(v, visitProgram(_));
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitFuncDecl(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("foo", Annotation::NONE)))
              .After(exp);
    exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1))).After(exp);
    exp = EXPECT_CALL(v, visitVarAssignment(_)).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(v, visitIdentifier(IdentifierEq("a", Annotation::NONE)))
              .After(exp);
    exp = EXPECT_CALL(v, visitByteLiteral(ByteLiteralEq(2))).After(exp);
    exp = EXPECT_CALL(v, visitVarRef(_)).After(exp);
    exp = EXPECT_CALL(
              v, visitIdentifier(IdentifierEq("color", Annotation::NONE)))
              .After(exp);

    visitAST(ast, v);
}

TEST_F(NAME, command_containing_builtin_in_middle)
{
    cmdIndex.addCommand(new cmd::Command(
        nullptr,
        "set effect constant boolean",
        "",
        cmd::Command::Type::Void,
        {}));
    cmdIndex.addCommand(new cmd::Command(
        nullptr,
        "set effect constant float",
        "",
        cmd::Command::Type::Void,
        {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test", "set
effect constant float RingsFX, \"shrink\", BlackHoleFunnel(0).shrink#\n",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, command_variable_name)
{
    cmdIndex.addCommand(
        new cmd::Command(nullptr, "text", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test", "text$ as string", matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, builtin_keyword_variable_name_1)
{
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test", "string$ as string", matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, builtin_keyword_variable_name_2)
{
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test", "string# as float", matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, command_variable_name_1)
{
    cmdIndex.addCommand(
        new cmd::Command(nullptr, "command", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test", "command$ as string", matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, command_variable_name_2)
{
    cmdIndex.addCommand(
        new cmd::Command(nullptr, "command", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test", "command# as float", matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, command_with_same_name_as_keyword)
{
    cmdIndex.addCommand(
        new cmd::Command(nullptr, "loop", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test", "do\nloop", matcher);
    ASSERT_THAT(ast, NotNull());
}
*/
