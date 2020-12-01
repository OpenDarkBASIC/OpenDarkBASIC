#include <gmock/gmock.h>
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME db_loop_do

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odb;

TEST_F(NAME, infinite_loop)
{
    ASSERT_THAT(driver->parseString("do\nfoo()\nloop\n"), IsTrue());
}

TEST_F(NAME, empty_infinite_loop)
{
    ASSERT_THAT(driver->parseString("do\nloop\n"), IsTrue());
}

TEST_F(NAME, break_from_loop)
{
    ASSERT_THAT(driver->parseString("do\nbreak\nloop\n"), IsTrue());
}
