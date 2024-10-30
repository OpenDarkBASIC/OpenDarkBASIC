#include "odb-compiler/tests/DBParserHelper.hpp"
#include "odb-util/tests/LogHelper.hpp"

#include "gmock/gmock.h"

extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_as_type

using namespace testing;

struct NAME : DBParserHelper, LogHelper, Test
{
};

TEST_F(NAME, self_referential_1)
{
    const char* source =
        "a AS = a\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
}

TEST_F(NAME, self_referential_2)
{
    const char* source =
        "a AS = a + 2\n";
    ASSERT_THAT(parse(source), Eq(0)) << log().text;
    ASSERT_THAT(semantic(&semantic_type_check), Eq(0)) << log().text;
}
