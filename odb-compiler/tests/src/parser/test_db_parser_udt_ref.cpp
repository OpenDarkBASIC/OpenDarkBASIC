#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"

#define NAME db_parser_udt_ref

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

TEST_F(NAME, access_members)
{
    ast = driver->parseString("test",
        "v.x# = 2.3\n"
        "v.y# = 5.4\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, udt_in_udt)
{
    ast = driver->parseString("test",
        "p.pos.x# = 2.3\n"
        "p.pos.y# = 5.4\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, udt_array)
{
    ast = driver->parseString("test",
        "vecs(2).x# = 2.3\n"
        "vecs(2).y# = 5.4\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, udt_array_with_udt_arrays)
{
    ast = driver->parseString("test",
        "p(1).pos(4).x# = 2.3\n"
        "p(1).pos(4).y# = 5.4\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, udt_func_return_value)
{
    ast = driver->parseString("test",
        "func().x# = 2.3\n"
        "func().y# = 5.4\n");
    ASSERT_THAT(ast, NotNull());
}
