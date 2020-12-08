#include "odb-compiler/ast/Node.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"

#define NAME db_keyword

using namespace testing;
using namespace odb;

class NAME : public ParserTestHarness
{
public:
};

/*
TEST_F(NAME, print_command)
{
    kwIndex.addKeyword({"print", "", "", {}, std::nullopt});
    matcher.updateFromIndex(&kwIndex);
    ASSERT_THAT(driver->parseString(
        "print \"hello world\"\n"), IsTrue());
}

TEST_F(NAME, command_with_spaces)
{
    kwIndex.addKeyword({"make object sphere", "", "", {}, std::nullopt});
    matcher.updateFromIndex(&kwIndex);
    ASSERT_THAT(driver->parseString(
        "make object sphere 1, 10\n"), IsTrue());
}

TEST_F(NAME, randomize_timer)
{
    kwIndex.addKeyword({"randomize", "", "", {}, std::nullopt});
    kwIndex.addKeyword({"timer", "", "", {}, {Keyword::Type::Integer}});
    matcher.updateFromIndex(&kwIndex);
    ASSERT_THAT(driver->parseString(
        "randomize timer()\n"), IsTrue());
}

TEST_F(NAME, load_3d_sound)
{
    kwIndex.addKeyword({"load 3dsound", "", "", {}, std::nullopt});
    matcher.updateFromIndex(&kwIndex);
    ASSERT_THAT(driver->parseString(
        "load 3dsound \"howl.wav\",s\n"), IsTrue());
}

TEST_F(NAME, command_with_variable_args)
{
    kwIndex.addKeyword({"clone sound", "", "", {}, std::nullopt});
    matcher.updateFromIndex(&kwIndex);
    ASSERT_THAT(driver->parseString(
        "clone sound s,2\n"), IsTrue());
}

TEST_F(NAME, command_with_spaces_as_argument_to_command_with_spaces)
{
    kwIndex.addKeyword({"make object sphere", "", "", {}, std::nullopt});
    kwIndex.addKeyword({"get ground height", "", "", {}, {Keyword::Type::Integer}});
    matcher.updateFromIndex(&kwIndex);
    ASSERT_THAT(driver->parseString(
        "make object sphere get ground height(1, x, y), 10\n"), IsTrue());
}

TEST_F(NAME, keyword_starting_with_builtin)
{
    // "loop" is a builtin keyword
    kwIndex.addKeyword({"loop", "", "", {}, std::nullopt});
    kwIndex.addKeyword({"loop sound", "", "", {}, std::nullopt});
    matcher.updateFromIndex(&kwIndex);
    ASSERT_THAT(driver->parseString("loop sound 1\n"), IsTrue());
}

TEST_F(NAME, builtin_shadowing_keyword)
{
    // "loop" is a builtin keyword
    kwIndex.addKeyword({"loop", "", "", {}, std::nullopt});
    kwIndex.addKeyword({"loop sound", "", "", {}, std::nullopt});
    matcher.updateFromIndex(&kwIndex);
    ASSERT_THAT(driver->parseString("do : foo() : loop"), IsTrue());
}

TEST_F(NAME, multiple_similar_keywords_with_spaces)
{
    // "loop" is a builtin keyword
    kwIndex.addKeyword({"set object", "", "", {}, std::nullopt});
    kwIndex.addKeyword({"set object speed", "", "", {}, std::nullopt});
    matcher.updateFromIndex(&kwIndex);
    ASSERT_THAT(driver->parseString("set object speed 1, 10\n"), IsTrue());
}

TEST_F(NAME, multiple_similar_keywords_with_spaces_2)
{
    kwIndex.addKeyword({"SET OBJECT AMBIENT", "", "", {}});
    kwIndex.addKeyword({"SET OBJECT COLLISION ON", "", "", {}});
    kwIndex.addKeyword({"SET OBJECT COLLISION OFF", "", "", {}});
    kwIndex.addKeyword({"SET OBJECT COLLISION TO BOXES", "", "", {}});
    kwIndex.addKeyword({"SET OBJECT", "", "", {}});
    kwIndex.addKeyword({"set object collision off", "", "", {}});
    matcher.updateFromIndex(&kwIndex);
    ASSERT_THAT(driver->parseString("set object collision off 1\n"), IsTrue());
}

TEST_F(NAME, incomplete_keyword_at_end_of_file)
{
    kwIndex.addKeyword({"color object", "", "", {}});
    matcher.updateFromIndex(&kwIndex);
    ASSERT_THAT(driver->parseString(
        "function foo()\n"
        "    a = 2\n"
        "endfunction color"), IsTrue());
}

TEST_F(NAME, keywords_with_type)
{
    kwIndex.addKeyword({"get dir$", "", "", {}, {Keyword::Type::Integer}});
    matcher.updateFromIndex(&kwIndex);
    ASSERT_THAT(driver->parseString(
        "OriginalDirectory$ = get dir$()"), IsTrue());
}

TEST_F(NAME, keyword_containing_builtin_in_middle)
{
    kwIndex.addKeyword({"set effect constant boolean", "", "", {}});
    kwIndex.addKeyword({"set effect constant float", "", "", {}});
    matcher.updateFromIndex(&kwIndex);
    ASSERT_THAT(driver->parseString(
        "set effect constant float RingsFX, \"shrink\", BlackHoleFunnel(0).shrink#\n"), IsTrue());
}

TEST_F(NAME, keyword_variable_name)
{
    kwIndex.addKeyword({"text", "", "", {}});
    matcher.updateFromIndex(&kwIndex);
    ASSERT_THAT(driver->parseString(
        "text$ as string"), IsTrue());
}

TEST_F(NAME, builtin_keyword_variable_name)
{
    matcher.updateFromIndex(&kwIndex);
    ASSERT_THAT(driver->parseString(
        "string$ as string"), IsTrue());
}
*/
