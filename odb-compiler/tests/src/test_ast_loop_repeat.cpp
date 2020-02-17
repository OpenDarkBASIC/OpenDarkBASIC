#include <gmock/gmock.h>
#include "odbc/Driver.hpp"
#include "odbc/ASTNode.hpp"
#include "odbc/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME ast_repeat

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odbc;

TEST_F(NAME, infinite_loop)
{
    EXPECT_THAT(driver->parseString("repeat\nfoo()\nuntil cond\n"), IsTrue());

    std::ofstream out("out.dot");
    ast::dumpToDOT(out, driver->getAST());
}
