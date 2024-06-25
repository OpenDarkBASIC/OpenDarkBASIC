#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-sdk/tests/LogHelper.hpp"

#include <gmock/gmock.h>

extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_initialize_undeclared_variables

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, test)
{
    const char* source = "";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    EXPECT_THAT(
        semantic_check_run(
            &semantic_type_check_and_cast, &ast, plugins, &cmds, "test", src),
        Eq(-1));
}
