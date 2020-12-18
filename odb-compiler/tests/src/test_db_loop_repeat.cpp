#include "gmock/gmock.h"
#include "odb-compiler/ast/Loop.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/keywords/Keyword.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"

#define NAME db_loop_repeat

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odb;

TEST_F(NAME, infinite_loop)
{
    ast = driver->parseString("test", "repeat\nfoo()\nuntil cond\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, empty_loop)
{
    ast = driver->parseString("test", "repeat\nuntil cond\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, break_from_loop)
{
    ast = driver->parseString("test", "repeat\nbreak\nuntil cond\n");
    ASSERT_THAT(ast, NotNull());
}
