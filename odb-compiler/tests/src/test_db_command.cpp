#include <gmock/gmock.h>
#include "odbc/parsers/db/Driver.hpp"
#include "odbc/ast/Node.hpp"
#include "odbc/tests/ParserTestHarness.hpp"

#define NAME db_command

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odbc;

TEST_F(NAME, print_command)
{
    db.addKeyword({"print", "", {}});
    matcher.updateFromDB(&db);
    ASSERT_THAT(driver->parseString(
        "print \"hello world\"\n"), IsTrue());
}

TEST_F(NAME, command_with_spaces)
{
    ASSERT_THAT(driver->parseString(
        "make object sphere 1, 10\n"), IsTrue());
}

TEST_F(NAME, randomize_timer)
{
    ASSERT_THAT(driver->parseString(
        "randomize timer()\n"), IsTrue());
}

TEST_F(NAME, load_3d_sound)
{
    ASSERT_THAT(driver->parseString(
        "load 3dsound \"howl.wav\",s\n"), IsTrue());
}

TEST_F(NAME, command_with_variable_args)
{
    ASSERT_THAT(driver->parseString(
        "clone sound s,2\n"), IsTrue());
}
