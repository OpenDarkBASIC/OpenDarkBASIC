#include "odb-compiler/tests/DBParserHelper.hpp"
#include <filesystem>

extern "C" {
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/sdk/type.h"
#include "odb-sdk/utf8.h"
}

void
DBParserHelper::SetUp()
{
    plugin_list_init(&plugins);
    cmd_list_init(&cmds);
    db_parser_init(&p);
    memset(&src, 0, sizeof(src));
    ast_init(&ast);
}

void
DBParserHelper::TearDown()
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
    plugin_list_deinit(&plugins);
}

int
DBParserHelper::parse(const char* code)
{
    if (src.text.data)
        db_source_close(&src);
    if (db_source_open_string(&src, cstr_utf8_view(code)) != 0)
        return -1;
    return db_parse(&p, &ast, "test", src, &cmds);
}

int
DBParserHelper::addCommand(
    type return_type, const char* name, std::initializer_list<type> param_types)
{
    cmd_id cmd = cmd_list_add(
        &cmds, 0, return_type, cstr_utf8_view(name), empty_utf8_view());
    if (cmd < 0)
        return cmd;

    for (type type : param_types)
        if (cmd_add_param(
                &cmds,
                cmd,
                type,
                CMD_PARAM_IN,
                cstr_utf8_view(type_to_db_name(type)))
            < 0)
        {
            cmd_list_erase(&cmds, cmd);
            return -1;
        }

    return cmd;
}
int
DBParserHelper::addCommand(const char* name)
{
    return addCommand(TYPE_VOID, name);
}
int
DBParserHelper::addCommand(type return_type, const char* name)
{
    return addCommand(return_type, name, {});
}
