#include "odb-compiler/ast/SourceLocation.hpp"
#include "odb-compiler/commands/Command.hpp"
#include "odb-compiler/parsers/db/Driver.hpp"
#include "odb-compiler/tests/ASTMatchers.hpp"
#include "odb-compiler/tests/ASTMockVisitor.hpp"
#include "odb-compiler/tests/ParserTestHarness.hpp"
#include "odb-sdk/Str.hpp"
#include <fstream>
#include <tuple>

#define NAME db_remarks

using namespace testing;
using namespace odb;
using namespace ast;

class NAME : public ParserTestHarness
{
public:
    void doTestWithRemark(const std::string& s)
    {
        using Annotation = ast::AnnotatedSymbol::Annotation;

        std::vector<std::string> variants{s, s, s};
        str::replaceAll(variants[0], "%remstart", "remstart");
        str::replaceAll(variants[1], "%remstart", "/*");
        str::replaceAll(variants[2], "%remstart", "/*");

        str::replaceAll(variants[0], "%remend", "remend");
        str::replaceAll(variants[1], "%remend", "*/");
        str::replaceAll(variants[2], "%remend", "*/");

        str::replaceAll(variants[0], "%rem", "rem");
        str::replaceAll(variants[1], "%rem", "//");
        str::replaceAll(variants[2], "%rem", "`");

        for (int i = 0; i != 3; ++i)
        {
            ast = driver->parseString("test",
                (variants[i] + "\nfoo()\n").c_str());
            ASSERT_THAT(ast, NotNull());

            StrictMock<ASTMockVisitor> v;
            Expectation exp;
            exp = EXPECT_CALL(v, visitBlock(BlockStmntCountEq(1)));
            exp = EXPECT_CALL(v, visitFuncCallStmnt(_)).After(exp);
            exp = EXPECT_CALL(v, visitAnnotatedSymbol(AnnotatedSymbolEq(Annotation::NONE, "foo"))).After(exp);

            ast->accept(&v);
        }
    }
};

TEST_F(NAME, some_remarks)
{
    doTestWithRemark(
        "%rem This is a comment\n"
        "    %rem this is also a comment\n"
        "%rem\n"
        "%rem    \n"
        "%rem\t\n"
        "   %rem\n"
        "\t%rem\n"
        "   %rem\t\n"
        "\t%rem\t\n"
        "%rem %rem %rem\n");
}

TEST_F(NAME, remarks_with_empty_lines)
{
    doTestWithRemark(
        "\n\n\n"
        "%rem This is a comment\n"
        "\n\n\n"
        "%rem\n"
        "%rem    \n"
        "%rem\t\n"
        "\n\n\n"
        "\t%rem\n"
        "   %rem\t\n"
        "\t%rem\t\n"
        "%rem %rem %rem\n"
        "\n\n\n");
}

TEST_F(NAME, remarks_empty_line_command)
{
    doTestWithRemark(
        "%rem some remark\n"
        "\n");
}

TEST_F(NAME, remstart_remend)
{
    doTestWithRemark(
        "%remstart\n"
        "this is a comment\n"
        "%remend\n");
}

TEST_F(NAME, remstart_remend_indentation)
{
    doTestWithRemark(
        "\n"
        "    %remstart\n"
        "    flags:\n"
        "    0 - [USER]\n"
        "    1 - [INFO]\n"
        "    2 - [ERROR]\n\n\n"
        "    3 - [SEVERE]\n"
        "    4 - [DEBUG}\n"
        "    %remend\n");
}

TEST_F(NAME, different_start_end_tokens_1)
{
    ast = driver->parseString("test",
        "remstart this is a comment\nlol\n*/\nfoo()\n");
    ASSERT_THAT(ast, IsNull());
}

TEST_F(NAME, different_start_end_tokens_2)
{
    ast = driver->parseString("test",
        "/* this is a comment\nlol\nremend\nfoo()\n");
    ASSERT_THAT(ast, IsNull());
}
