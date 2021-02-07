#include "gmock/gmock.h"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"

#define NAME db_parser_select

using namespace testing;

class NAME : public ParserTestHarness
{
public:
};

using namespace odb;
using namespace ast;

TEST_F(NAME, empty_select_endselect)
{
    ast = driver->parseString("test",
        "select var\nendselect\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, select_with_one_empty_case)
{
    ast = driver->parseString("test",
        "select var1\ncase var2\nendcase\nendselect\n");
}

TEST_F(NAME, select_with_two_empty_cases)
{
    ast = driver->parseString("test",
        "select var1\n"
        "    case var2\n"
        "    endcase\n"
        "    case var3\n"
        "    endcase\n"
        "endselect\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, select_with_empty_case_and_default_case)
{
    ast = driver->parseString("test",
        "select var1\n"
        "    case var2\n"
        "    endcase\n"
        "    case default\n"
        "    endcase\n"
        "endselect\n");
    ASSERT_THAT(ast, NotNull());
}

TEST_F(NAME, select_case_with_body)
{
    ast = driver->parseString("test",
        "select var1\n"
        "    case 1\n"
        "        foo()\n"
        "    endcase\n"
        "    case 2\n"
        "        bar()\n"
        "    endcase\n"
        "    case default\n"
        "        baz()\n"
        "    endcase\n"
        "endselect\n");
    ASSERT_THAT(ast, NotNull());
}
