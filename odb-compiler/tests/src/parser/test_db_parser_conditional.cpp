#include "odb-compiler/ast/Conditional.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"

#define NAME db_parser_conditional

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odb;

TEST_F(NAME, simple_if_then)
{
    ast = driver->parse("test",
        "if a = 3 then foo()\n",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, simple_if_then_else)
{
    ast = driver->parse("test",
        "if a = 3 then foo() else bar()\n",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, empty_then_works_when_theres_an_else)
{
    ast = driver->parse("test",
        "if a = 3 then else bar()\n",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, empty_then_doesnt_work_alone)
{
    ast = driver->parse("test",
        "if a = 3 then\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, multi_line_if)
{
    ast = driver->parse("test",
        "if a = 3\n"
        "    foo1()\n"
        "    foo2()\n"
        "endif\n",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, empty_multi_line_if)
{
    ast = driver->parse("test",
        "if a = 3\n"
        "endif\n",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, multi_line_if_else)
{
    ast = driver->parse("test",
        "if a = 3\n"
        "    foo1()\n"
        "    foo2()\n"
        "else\n"
        "    bar1()\n"
        "    bar2()\n"
        "endif\n",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, empty_multi_line_if_else)
{
    ast = driver->parse("test",
        "if a = 3\n"
        "else\n"
        "endif\n",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, multi_line_if_elseif)
{
    ast = driver->parse("test",
        "if a = 3\n"
        "    foo1()\n"
        "    foo2()\n"
        "elseif b = 4\n"
        "    bar1()\n"
        "    bar2()\n"
        "elseif c = 5\n"
        "    baz1()\n"
        "    baz2()\n"
        "endif\n",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, empty_multi_line_if_elseif)
{
    ast = driver->parse("test",
        "if a = 3\n"
        "elseif b = 4\n"
        "elseif c = 5\n"
        "endif\n",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, multi_line_if_elseif_else)
{
    ast = driver->parse("test",
        "if a = 3\n"
        "    foo1()\n"
        "    foo2()\n"
        "elseif b = 4\n"
        "    bar1()\n"
        "    bar2()\n"
        "else\n"
        "    baz1()\n"
        "    baz2()\n"
        "endif\n",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, empty_multi_line_if_elseif_else)
{
    ast = driver->parse("test",
        "if a = 3\n"
        "elseif b = 4\n"
        "else\n"
        "endif\n",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, multi_line_if_elseif_else_nested)
{
    ast = driver->parse("test",
        "if a = 3\n"
        "    foo1()\n"
        "    if d = 12\n"
        "        foo3()\n"
        "    elseif e = 35\n"
        "        bar3()\n"
        "    endif\n"
        "elseif b = 4\n"
        "    bar1()\n"
        "    bar2()\n"
        "else\n"
        "    baz1()\n"
        "    baz2()\n"
        "endif\n",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, empty_multi_line_if_elseif_else_nested)
{
    ast = driver->parse("test",
        "if a = 3\n"
        "    if d = 12\n"
        "    elseif e = 35\n"
        "    endif\n"
        "elseif b = 4\n"
        "else\n"
        "endif\n",
        matcher);
    ASSERT_THAT(ast, NotNull());
}
