#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-compiler/tests/LogHelper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_and_cast_binop_pow

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, exponent_truncated_from_double)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_DOUBLE});
    ASSERT_THAT(parse("print 2.0f ^ 2.0"), Eq(0));
    EXPECT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
    EXPECT_THAT(
        log(),
        LogEq("test:1:14: warning: Exponent value is truncated when converting "
              "from DOUBLE to FLOAT.\n"
              " 1 | print 2.0f ^ 2.0\n"
              "   |       >~~~ ^ ~~< DOUBLE\n"
              "   |       FLOAT\n"
              "   = note: The exponent is always converted to the same type as "
              "the base when using floating point exponents.\n"
              "   = note: The exponent can be a INTEGER, FLOAT or DOUBLE.\n"));
}

TEST_F(NAME, exponent_cast_to_integer)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_DOUBLE});
    ASSERT_THAT(parse("print 2.0 ^ 2"), Eq(0));
    EXPECT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
    EXPECT_THAT(log(), LogEq(""));
}

TEST_F(NAME, exponent_strange_conversion)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_DOUBLE});
    ASSERT_THAT(parse("print 2.0 ^ true"), Eq(0));
    EXPECT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
    EXPECT_THAT(
        log(),
        LogEq("test:1:13: warning: Strange conversion of exponent value from "
              "BOOLEAN to INTEGER.\n"
              " 1 | print 2.0 ^ true\n"
              "   |       >~~ ^ ~~~< BOOLEAN\n"
              "   = note: The exponent can be a INTEGER, FLOAT or DOUBLE.\n"));
}

TEST_F(NAME, exponent_truncated_from_dword)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_DOUBLE});
    ASSERT_THAT(parse("print 2.0 ^ 4294967295"), Eq(0));
    EXPECT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
    EXPECT_THAT(
        log(),
        LogEq("test:1:13: warning: Exponent value is truncated when converting "
              "from DWORD to INTEGER.\n"
              " 1 | print 2.0 ^ 4294967295\n"
              "   |       >~~ ^ ~~~~~~~~~< DWORD\n"
              "   = note: INTEGER is the largest possible integral type for "
              "exponents.\n"
              "   = note: The exponent can be a INTEGER, FLOAT or DOUBLE.\n"));
}

TEST_F(NAME, exponent_truncated_from_long_integer)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_DOUBLE});
    ASSERT_THAT(parse("print 2.0 ^ 99999999999999"), Eq(0));
    EXPECT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
    EXPECT_THAT(
        log(),
        LogEq("test:1:13: warning: Exponent value is truncated when converting "
              "from LONG INTEGER to INTEGER.\n"
              " 1 | print 2.0 ^ 99999999999999\n"
              "   |       >~~ ^ ~~~~~~~~~~~~~< LONG INTEGER\n"
              "   = note: INTEGER is the largest possible integral type for "
              "exponents.\n"
              "   = note: The exponent can be a INTEGER, FLOAT or DOUBLE.\n"));
}

TEST_F(NAME, exponent_invalid_type)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_DOUBLE});
    ASSERT_THAT(parse("print 2.0 ^ \"oops\""), Eq(0));
    EXPECT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(-1));
    EXPECT_THAT(
        log(),
        LogEq("test:1:13: error: Incompatible exponent type STRING can't be "
              "converted to INTEGER.\n"
              " 1 | print 2.0 ^ \"oops\"\n"
              "   |       >~~ ^ ~~~~~< STRING\n"
              "   = note: The exponent can be a INTEGER, FLOAT or DOUBLE.\n"));
}
