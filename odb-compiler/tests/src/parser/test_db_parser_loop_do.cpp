#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"

#define NAME db_parser_loop_do

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odb;

TEST_F(NAME, infinite_loop)
{
    ast = driver->parse("test", "do\nfoo()\nloop\n", matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, empty_infinite_loop)
{
    ast = driver->parse("test", "do\nloop\n", matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, exit_from_loop)
{
    ast = driver->parse("test", "do\nexit\nloop\n", matcher);
    ASSERT_THAT(ast, NotNull());
}
