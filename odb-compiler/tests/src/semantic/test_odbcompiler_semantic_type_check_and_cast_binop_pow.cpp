#include "odb-compiler/tests/DBParserHelper.hpp"
extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_and_cast_binop_pow

using namespace testing;

struct NAME : public DBParserHelper
{
};

TEST_F(NAME, exponent_truncated_from_double)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_DOUBLE});
    ASSERT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
    ASSERT_THAT(parse("print 2.0f ^ 2.0"), Eq(0));
    ASSERT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
}

TEST_F(NAME, exponent_cast_to_integer)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_DOUBLE});
    ASSERT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
    ASSERT_THAT(parse("print 2.0 ^ 2"), Eq(0));
    ASSERT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
}

TEST_F(NAME, exponent_strange_conversion)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_DOUBLE});
    ASSERT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
    ASSERT_THAT(parse("print 2.0 ^ true"), Eq(0));
    ASSERT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
}

TEST_F(NAME, exponent_truncated_from_dword)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_DOUBLE});
    ASSERT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
    ASSERT_THAT(parse("print 2.0 ^ 4294967295"), Eq(0));
    ASSERT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
}

TEST_F(NAME, exponent_truncated_from_long_integer)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_DOUBLE});
    ASSERT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
    ASSERT_THAT(parse("print 2.0 ^ 99999999999999"), Eq(0));
    ASSERT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
}

TEST_F(NAME, exponent_invalid_type)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_DOUBLE});
    ASSERT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
    ASSERT_THAT(parse("print 2.0 ^ \"oops\""), Eq(0));
    ASSERT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(-1));
}

