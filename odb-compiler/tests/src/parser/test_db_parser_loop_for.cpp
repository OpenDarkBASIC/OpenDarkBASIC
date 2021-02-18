#include "gmock/gmock.h"
#include "odb-compiler/ast/Loop.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/commands/Command.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"

#define NAME db_parser_loop_for

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odb;

TEST_F(NAME, count_to_5)
{
    ast = driver->parse("test", "for n=1 to 5\nfoo(n)\nnext n\n", matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, empty_loop)
{
    ast = driver->parse("test", "for n=1 to 5\nnext n\n", matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, exit_from_loop)
{
    ast = driver->parse("test", "for n=1 to 5\nexit\nnext n\n", matcher);
    ASSERT_THAT(ast, NotNull());
}
