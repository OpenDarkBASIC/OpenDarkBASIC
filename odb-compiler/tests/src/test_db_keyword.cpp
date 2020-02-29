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
    db.addKeyword({"loop", "", {}, false});
    db.addKeyword({"loop sound", "", {}, false});
    matcher.updateFromDB(&db);
    ASSERT_THAT(driver->parseString("loop sound 1\n"), IsTrue());
}

TEST_F(NAME, builtin_shadowing_keyword)
{
    // "loop" is a builtin keyword
    db.addKeyword({"loop", "", {}, false});
    db.addKeyword({"loop sound", "", {}, false});
    matcher.updateFromDB(&db);
    ASSERT_THAT(driver->parseString("do : foo() : loop"), IsTrue());
}

TEST_F(NAME, multiple_similar_keywords_with_spaces)
{
    // "loop" is a builtin keyword
    db.addKeyword({"set object", "", {}, false});
    db.addKeyword({"set object speed", "", {}, false});
    matcher.updateFromDB(&db);
    ASSERT_THAT(driver->parseString("set object speed 1, 10\n"), IsTrue());
}

TEST_F(NAME, incomplete_keyword_at_end_of_file)
{
    db.addKeyword({"color object", "", {}});
    matcher.updateFromDB(&db);
    ASSERT_THAT(driver->parseString(
        "function foo()\n"
        "    a = 2\n"
        "endfunction color"), IsTrue());
}

TEST_F(NAME, keywords_with_type)
{
    db.addKeyword({"get dir$", "", {}, true});
    matcher.updateFromDB(&db);
    ASSERT_THAT(driver->parseString(
        "OriginalDirectory$ = get dir$()"), IsTrue());
}

TEST_F(NAME, keyword_containing_builtin_in_middle)
{
    db.addKeyword({"set effect constant boolean", "", {}});
    db.addKeyword({"set effect constant float", "", {}});
    matcher.updateFromDB(&db);
    ASSERT_THAT(driver->parseString(
        "set effect constant float RingsFX, \"shrink\", BlackHoleFunnel(0).shrink#\n"), IsTrue());
}
