#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include <gmock/gmock.h>

extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_resolve_cmd_overloads_warnings

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, float_accepts_byte)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_F32});
    ASSERT_THAT(parse("print 5"), Eq(0));
    EXPECT_THAT(semantic(&semantic_resolve_cmd_overloads), Eq(0));
    EXPECT_THAT(
        log(),
        LogEq("test:1:7: warning: Implicit conversion of argument 1 from BYTE "
              "to FLOAT in command call.\n"
              " 1 | print 5\n"
              "   |       ^ BYTE\n"
              "   = note: Calling command: PRINT FLOAT AS FLOAT  [test]\n"));
}

TEST_F(NAME, float_accepts_byte_2)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_U8, TYPE_F32, TYPE_U8});
    ASSERT_THAT(parse("print 5, 6, 7"), Eq(0));
    EXPECT_THAT(semantic(&semantic_resolve_cmd_overloads), Eq(0));
    EXPECT_THAT(
        log(),
        LogEq("test:1:10: warning: Implicit conversion of argument 2 from BYTE "
              "to FLOAT in command call.\n"
              " 1 | print 5, 6, 7\n"
              "   |          ^ BYTE\n"
              "   = note: Calling command: PRINT BYTE AS BYTE, FLOAT AS FLOAT, BYTE AS BYTE  [test]\n"));
}

TEST_F(NAME, integer_accepts_float_with_warning)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_I32});
    ASSERT_THAT(parse("print 5.5f"), Eq(0));
    EXPECT_THAT(semantic(&semantic_resolve_cmd_overloads), Eq(0));
    EXPECT_THAT(
        log(),
        LogEq(
            "test:1:7: warning: Argument 1 is truncated in conversion "
            "from FLOAT to INTEGER in command call.\n"
            " 1 | print 5.5f\n"
            "   |       ^~~< FLOAT\n"
            "   = note: Calling command: PRINT INTEGER AS INTEGER  [test]\n"));
}

TEST_F(NAME, dword_accepts_integer_with_warning)
{
    addCommand(TYPE_VOID, "PRINT", {TYPE_STRING});
    addCommand(TYPE_VOID, "PRINT", {TYPE_U32});
    addCommand(TYPE_VOID, "PRINT", {TYPE_F32});
    ASSERT_THAT(parse("print n"), Eq(0));
    EXPECT_THAT(semantic(&semantic_resolve_cmd_overloads), Eq(0));
    EXPECT_THAT(
        log(),
        LogEq("test:1:7: warning: Argument 1 is truncated in conversion "
              "from INTEGER to DWORD in command call.\n"
              " 1 | print n\n"
              "   |       ^ INTEGER\n"
              "   = note: Calling command: PRINT DWORD AS INTEGER  [test]\n"));
}
