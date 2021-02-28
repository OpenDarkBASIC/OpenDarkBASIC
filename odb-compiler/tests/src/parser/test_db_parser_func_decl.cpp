#include "odb-compiler/ast/Block.hpp"
#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"

#define NAME db_parser_func_decl

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odb;

TEST_F(NAME, empty_function)
{
    ast = driver->parse("test",
        "function myfunc()\n"
        "endfunction\n",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, empty_function_with_return_argument)
{
    ast = driver->parse("test",
        "function myfunc()\n"
        "endfunction a+b\n",
        matcher);
}

TEST_F(NAME, function)
{
    ast = driver->parse("test",
        "function myfunc()\n"
        "    foo()\n"
        "endfunction\n",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, function_two_args)
{
    ast = driver->parse("test",
        "function myfunc(a, b)\n"
        "    foo()\n"
        "endfunction\n",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, function_retval)
{
    ast = driver->parse("test",
        "function myfunc()\n"
        "    foo()\n"
        "endfunction a+b\n",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, function_exitfunction)
{
    ast = driver->parse("test",
        "function myfunc()\n"
        "    exitfunction c+d\n"
        "endfunction a+b\n",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, function_can_be_annotated_1)
{
    ast = driver->parse("test",
        "function myfunc$()\n"
        "endfunction a+b\n",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, function_can_be_annotated_2)
{
    ast = driver->parse("test",
        "function myfunc#()\n"
        "endfunction a+b\n",
        matcher);
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, functions_may_not_start_with_integers)
{
    ast = driver->parse("test",
        "function 3myfunc()\n"
        "endfunction a+b\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, functions_may_not_start_with_dollar)
{
    ast = driver->parse("test",
        "function $myfunc()\n"
        "endfunction a+b\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, functions_may_not_start_with_hash)
{
    ast = driver->parse("test",
        "function #myfunc()\n"
        "endfunction a+b\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, functions_may_not_be_a_single_hash)
{
    ast = driver->parse("test",
        "function #()\n"
        "endfunction a+b\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, functions_may_not_be_a_single_dollar)
{
    ast = driver->parse("test",
        "function $()\n"
        "endfunction a+b\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, functions_may_not_have_spaces)
{
    ast = driver->parse("test",
        "function my func()\n"
        "endfunction a+b\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, functions_may_not_shadow_keywords)
{
    ast = driver->parse("test",
        "function function()\n"
        "endfunction a+b\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, functions_may_not_shadow_commands)
{
    cmdIndex.addCommand(new cmd::Command(nullptr, "print", "", cmd::Command::Type::Void, {}));
    matcher.updateFromIndex(&cmdIndex);
    ast = driver->parse("test",
        "function print()\n"
        "endfunction a+b\n",
        matcher);
    ASSERT_THAT(ast, IsNull());
}
