#include "odb-compiler/tests/DBParserHelper.hpp"
extern "C" {
#include "odb-compiler/semantic/semantic.h"
}

#define NAME odbcompiler_semantic_type_check_and_cast_assignment

using namespace testing;

struct NAME : public DBParserHelper
{
};

TEST_F(NAME, exponent_truncated_from_double)
{
    const char* source = 
        "a = b + c\n"
        "b = a + c\n"
        "c = a + b\n";
    ASSERT_THAT(parse(source), Eq(0));
    ASSERT_THAT(
        semantic_type_check_and_cast.execute(
            &ast, &plugins, &cmds, "test", src),
        Eq(0));
}
