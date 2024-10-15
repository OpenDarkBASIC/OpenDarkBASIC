#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include <gmock/gmock.h>
extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_assignment_errors

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, invalid_assignment)
{
    const char* source
        = "a$ = \"lulul\"\n"
          "b = 2\n"
          "b = a\n";
    ASSERT_THAT(parse(source), Eq(0));
    EXPECT_THAT(runSemanticCheck(&semantic_type_check), Eq(-1));
    EXPECT_THAT(
        log(),
        LogEq("test:3:5: error: Cannot assign STRING to INTEGER. Types are "
              "incompatible.\n"
              " 3 | b = a\n"
              "   | ^ ^ ^ STRING\n"
              "   | INTEGER\n"
              "   = note: b was previously declared as INTEGER at test:2:1:\n"
              " 2 | b = 2\n"
              "   | ^ INTEGER\n"));
}
