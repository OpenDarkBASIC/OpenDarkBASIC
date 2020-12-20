#include "gmock/gmock.h"
#include "odb-compiler/ast/Loop.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/commands/Command.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"

#define NAME db_loop_do

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odb;

TEST_F(NAME, infinite_loop)
{
    ast = driver->parseString("test", "do\nfoo()\nloop\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, empty_infinite_loop)
{
    ast = driver->parseString("test", "do\nloop\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, break_from_loop)
{
    ast = driver->parseString("test", "do\nbreak\nloop\n");
    ASSERT_THAT(ast, NotNull());
}
