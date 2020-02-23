#include <gmock/gmock.h>
#include "odbc/parsers/db/Driver.hpp"
#include "odbc/ast/Node.hpp"
#include "odbc/tests/ParserTestHarness.hpp"

#define NAME db_keyword

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odbc;

TEST_F(NAME, print_command)
{
    db.addKeyword({"print", "", {}, false});
    matcher.updateFromDB(&db);
    ASSERT_THAT(driver->parseString(
        "print \"hello world\"\n"), IsTrue());
}

TEST_F(NAME, command_with_spaces)
{
    db.addKeyword({"make object sphere", "", {}, false});
    matcher.updateFromDB(&db);
    ASSERT_THAT(driver->parseString(
        "make object sphere 1, 10\n"), IsTrue());
}

TEST_F(NAME, randomize_timer)
{
    db.addKeyword({"randomize", "", {}, false});
    db.addKeyword({"timer", "", {}, true});
    matcher.updateFromDB(&db);
    ASSERT_THAT(driver->parseString(
        "randomize timer()\n"), IsTrue());
}

TEST_F(NAME, load_3d_sound)
{
    db.addKeyword({"load 3dsound", "", {}, false});
    matcher.updateFromDB(&db);
    ASSERT_THAT(driver->parseString(
        "load 3dsound \"howl.wav\",s\n"), IsTrue());
}

TEST_F(NAME, command_with_variable_args)
{
    db.addKeyword({"clone sound", "", {}, false});
    matcher.updateFromDB(&db);
    ASSERT_THAT(driver->parseString(
        "clone sound s,2\n"), IsTrue());
}

TEST_F(NAME, command_with_spaces_as_argument_to_command_with_spaces)
{
    db.addKeyword({"make object sphere", "", {}, false});
    db.addKeyword({"get ground height", "", {}, true});
    matcher.updateFromDB(&db);
    ASSERT_THAT(driver->parseString(
        "make object sphere get ground height(1, x, y), 10\n"), IsTrue());
}

TEST_F(NAME, keyword_starting_with_builtin)
{
    // "loop" is a builtin keyword
    db.addKeyword({"loop sound", "", {}, false});
    matcher.updateFromDB(&db);
    ASSERT_THAT(driver->parseString("loop sound\n"), IsTrue());
}

TEST_F(NAME, wat)
{
    db.addKeyword({"delete object", "", {}, false});
    matcher.updateFromDB(&db);
    ASSERT_THAT(driver->parseString("for e=1 to enemymax : delete object 100+(e*2)+1 : next e\n"), IsTrue());
}
