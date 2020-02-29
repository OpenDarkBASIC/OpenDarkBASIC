#include <gmock/gmock.h>
#include "odbc/parsers/db/Driver.hpp"
#include "odbc/ast/Node.hpp"
#include "odbc/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME db_loop_for

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odbc;

TEST_F(NAME, count_to_5)
{
    EXPECT_THAT(driver->parseString("for n=1 to 5\nfoo(n)\nnext n\n"), IsTrue());
}

TEST_F(NAME, empty_loop)
{
    EXPECT_THAT(driver->parseString("for n=1 to 5\nnext n\n"), IsTrue());
}

TEST_F(NAME, break_from_loop)
{
    EXPECT_THAT(driver->parseString("for n=1 to 5\nbreak\nnext n\n"), IsTrue());
}
