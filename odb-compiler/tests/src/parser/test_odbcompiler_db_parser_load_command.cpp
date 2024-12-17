#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/ast/ast.h"
}

#define NAME odbcompiler_db_parser_load_command

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, load_command)
{
    const char* source
        = "#LOAD COMMAND \"PRINT\", \"plugin.dll\", \"print_i32\", VOID, "
          "INTEGER\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
}

TEST_F(NAME, load_command_passing_udt)
{
    const char* source
        = "TYPE Vec2\n"
          "    x AS FLOAT\n"
          "    y AS FLOAT\n"
          "ENDTYPE\n"
          "#LOAD COMMAND \"print\", \"plugin.dll\", \"print_i32\", VOID, "
          "INTEGER\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
}
