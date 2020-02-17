#include <gmock/gmock.h>
#include "odbc/Driver.hpp"
#include "odbc/ASTNode.hpp"
#include "odbc/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME ast_while

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odbc;

TEST_F(NAME, infinite_loop)
{
    EXPECT_THAT(driver->parseString("while cond\nfoo()\nendwhile\n"), IsTrue());
}
