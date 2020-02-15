#include <gmock/gmock.h>
#include "odbc/Driver.hpp"
#include "odbc/ASTNode.hpp"
#define NAME op_binary
#include <fstream>

using namespace testing;

class NAME : public Test
{
public:
    void SetUp() override { driver = new odbc::Driver; }
    void TearDown() override { delete driver; }
    odbc::Driver* driver;
};

using namespace odbc;

TEST_F(NAME, add)
{
    ASSERT_THAT(driver->parseString("result = (3 + 5) * 8 + a^b*5\n"), IsTrue());
    std::ofstream out("out.dot");
    ast::dumpToDOT(out, driver->getAST());
}
