#include "odb-compiler/tests/DBParserHelper.hpp"

#include "gmock/gmock.h"

#define NAME odbcompiler_db_parser_remarks

using namespace testing;

struct NAME : DBParserHelper, Test
{
    void
    SetUp() override
    {
        addCommand(TYPE_VOID, "PRINT", {TYPE_INTEGER});
    }

    void
    replaceAll(
        std::string&       subject,
        const std::string& search,
        const std::string& replace)
    {
        size_t pos = 0;
        while ((pos = subject.find(search, pos)) != std::string::npos)
        {
            subject.replace(pos, search.length(), replace);
            pos += replace.length();
        }
    }

    void
    doTestWithRemark(const std::string& s)
    {
        std::vector<std::string> variants{s, s, s};
        replaceAll(variants[0], "%remstart", "remstart");
        replaceAll(variants[1], "%remstart", "/*");
        replaceAll(variants[2], "%remstart", "/*");

        replaceAll(variants[0], "%remend", "remend");
        replaceAll(variants[1], "%remend", "*/");
        replaceAll(variants[2], "%remend", "*/");

        replaceAll(variants[0], "%rem", "rem");
        replaceAll(variants[1], "%rem", "//");
        replaceAll(variants[2], "%rem", "`");

        for (int i = 0; i != 3; ++i)
        {
            ASSERT_THAT(parse((variants[i] + "\nprint 5\n").c_str()), Eq(0));
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

TEST_F(NAME, multiple_remstart_remend)
{
    doTestWithRemark(
        "%remstart\n"
        "this is a comment\n"
        "%remend\n"
        "%remstart\n"
        "this is another comment\n"
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
