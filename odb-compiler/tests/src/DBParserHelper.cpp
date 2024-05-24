#include "odb-compiler/tests/DBParserHelper.hpp"
#include <filesystem>

extern "C" {
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/sdk/cmd_list.h"
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

static const char*
cmd_param_type_to_db_name(cmd_param_type type)
{
    switch (type)
    {
        case CMD_PARAM_LONG: return "DOUBLE INTEGER";
        case CMD_PARAM_DWORD: return "DWORD";
        case CMD_PARAM_INTEGER: return "INTEGER";
        case CMD_PARAM_WORD: return "WORD";
        case CMD_PARAM_BYTE: return "BYTE";
        case CMD_PARAM_BOOLEAN: return "BOOLEAN";
        case CMD_PARAM_FLOAT: return "FLOAT";
        case CMD_PARAM_DOUBLE: return "DOUBLE";
        case CMD_PARAM_STRING: return "STRING";
        case CMD_PARAM_ARRAY: return "DIM";

        case CMD_PARAM_VOID:
        case CMD_PARAM_LABEL:
        case CMD_PARAM_DABEL:
        case CMD_PARAM_ANY:
        case CMD_PARAM_USER_DEFINED_VAR_PTR: break;
    }

    return "";
}

int
DBParserHelper::addCommand(
    cmd_param_type                        return_type,
    const char*                           name,
    std::initializer_list<cmd_param_type> param_types)
{
    cmd_id cmd = cmd_list_add(
        &cmds, 0, return_type, cstr_utf8_view(name), empty_utf8_view());
    if (cmd < 0)
        return cmd;

    for (cmd_param_type type : param_types)
        if (cmd_add_param(
                &cmds,
                cmd,
                type,
                CMD_PARAM_IN,
                cstr_utf8_view(cmd_param_type_to_db_name(type)))
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
    return addCommand(CMD_PARAM_VOID, name);
}
int
DBParserHelper::addCommand(cmd_param_type return_type, const char* name)
{
    return addCommand(return_type, name, {});
}
