#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"

#define NAME db_parser_loop_repeat

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odb;

TEST_F(NAME, infinite_loop)
{
    ast = driver->parse("test",
        "repeat\n"
        "    foo()\n"
        "until cond\n",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, empty_loop)
{
    ast = driver->parse("test",
        "repeat\n"
        "until cond\n",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, break_from_loop)
{
    ast = driver->parse("test",
        "repeat\n"
        "    exit\n"
        "until cond\n",
        matcher);
    ASSERT_THAT(ast, NotNull());
}
