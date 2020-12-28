#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"

#define NAME db_string_literal

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odb;
using namespace ast;

TEST_F(NAME, simple_string_assignment)
{
    ast = driver->parseString("test",
        "a = \"test\"");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, string_with_backslash)
{
    ast = driver->parseString("test",
        "a = \"test\\\"");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, string_with_path_backslashes)
{
    ast = driver->parseString("test",
        "a = \"path\\to\\file\"");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, string_append_filename_with_backslash)
{
    ast = driver->parseString("test",
        "if foo(\"maps\\\" + LevelEditor.name$) then bar(\"maps\\\" + LevelEditor.name$)");
    ASSERT_THAT(ast, NotNull());
}
