#include <gmock/gmock.h>
#include "odbc/Driver.hpp"
#include "odbc/ASTNode.hpp"
#include <fstream>

#define NAME ast_loop_for

using namespace testing;

class NAME : public Test
{
public:
    void SetUp() override { driver = new odbc::Driver; }
    void TearDown() override { delete driver; }
    odbc::Driver* driver;
};

using namespace odbc;

TEST_F(NAME, count_to_5)
{
    EXPECT_THAT(driver->parseString("for n=1 to 10 step 2\nfoo(n)\n next n\n"), IsTrue());

    std::ofstream out("out.dot");
    ast::dumpToDOT(out, driver->getAST());
}
