#include "odb-compiler/tests/DBParserTestHarness.hpp"
#include <filesystem>

extern "C" {
#include "odb-compiler/ast/ast.h"
#include "odb-compiler/ast/ast_export.h"
#include "odb-sdk/utf8.h"
}

void
DBParserTestHarness::SetUp()
{
    cmd_list_init(&cmds);
    db_parser_init(&p);
    memset(&src, 0, sizeof(src));
    ast_init(&ast);
}

void
DBParserTestHarness::TearDown()
{
    if (ast.node_count)
    {
#if defined(ODBCOMPILER_DOT_EXPORT)
        const testing::TestInfo* info
            = testing::UnitTest::GetInstance()->current_test_info();
        std::string filename = std::string("ast/") + info->test_suite_name()
                               + "__" + info->name() + ".dot";
        std::filesystem::create_directory("ast");
        ast_export_dot(&ast, cstr_utf8_view(filename.c_str()), &src, &cmds);
#endif
    }

    ast_deinit(&ast);
    db_parser_deinit(&p);
    if (src.text.data)
        db_source_close(&src);
    cmd_list_deinit(&cmds);
}

int
DBParserTestHarness::addCommand(enum cmd_param_type return_type, const char* name)
{
    return cmd_list_add(
        &cmds,
        0,
        return_type,
        cstr_utf8_view(name),
        empty_utf8_view(),
        empty_utf8_view());
}
int
DBParserTestHarness::addCommand(const char* name)
{
    return addCommand(CMD_PARAM_VOID, name);
}

int
DBParserTestHarness::parse(const char* code)
{
    if (src.text.data)
        db_source_close(&src);
    if (db_source_open_string(&src, cstr_utf8_view(code)) != 0)
        return -1;
    return db_parse(&p, &ast, src, &cmds);
}
