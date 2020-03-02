#include <gmock/gmock.h>
#include "odbc/parsers/db/Driver.hpp"
#include "odbc/ast/Node.hpp"
#include "odbc/tests/ParserTestHarness.hpp"

#define NAME db_goto

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odbc;
using namespace ast;

TEST_F(NAME, simple_goto)
{
    ASSERT_THAT(driver->parseString("goto label\n"), IsTrue());
}
