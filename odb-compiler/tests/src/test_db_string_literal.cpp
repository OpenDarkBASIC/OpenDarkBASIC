#include <gmock/gmock.h>
#include "odbc/parsers/db/Driver.hpp"
#include "odbc/ast/Node.hpp"
#include "odbc/tests/ParserTestHarness.hpp"

#define NAME db_string_literal

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odbc;
using namespace ast;

TEST_F(NAME, simple_string_assignment)
{
    ASSERT_THAT(driver->parseString("a = \"test\""), IsTrue());
}

TEST_F(NAME, string_with_backslash)
{
    ASSERT_THAT(driver->parseString("a = \"test\\\""), IsTrue());
}

TEST_F(NAME, string_with_path_backslashes)
{
    ASSERT_THAT(driver->parseString("a = \"path\\to\\file\""), IsTrue());
}

TEST_F(NAME, string_append_filename_with_backslash)
{
    ASSERT_THAT(driver->parseString("if foo(\"maps\\\" + LevelEditor.name$) then bar(\"maps\\\" + LevelEditor.name$)"), IsTrue());
}
