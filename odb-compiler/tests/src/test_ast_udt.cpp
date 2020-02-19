#include <gmock/gmock.h>
#include "odbc/parsers/db/Driver.hpp"
#include "odbc/ast/Node.hpp"
#include "odbc/tests/ParserTestHarness.hpp"
#include <fstream>

#define NAME ast_udt

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odbc;

TEST_F(NAME, declare_udt)
{
    EXPECT_THAT(driver->parseString(
        "type mytype\n"
        "    x# as float\n"
        "    y# as float\n"
        "endtype\n"), IsTrue());
}

TEST_F(NAME, declare_nested_udt)
{
    EXPECT_THAT(driver->parseString(
        "type vec2\n"
        "    x# as float\n"
        "    y# as float\n"
        "endtype\n"
        "type player\n"
        "    pos as vec2\n"
        "    health as integer\n"
        "endtype\n"), IsTrue());
}

TEST_F(NAME, declare_nested_udt_with_dims)
{
    EXPECT_THAT(driver->parseString(
        "type vec2\n"
        "    x# as float\n"
        "    y# as float\n"
        "endtype\n"
        "type player\n"
        "    dim pos(5) as vec2\n"
        "    health as integer\n"
        "endtype\n"), IsTrue());
}
