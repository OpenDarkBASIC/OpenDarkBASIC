#include "odb-compiler/tests/DBParserHelper.hpp"
#include <filesystem>

#include <gtest/gtest.h>

extern "C" {
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-compiler/semantic/symbol_table.h"
#include "odb-compiler/semantic/type.h"
#include "odb-util/mutex.h"
#include "odb-util/utf8.h"
}

DBParserHelper::DBParserHelper()
{
    plugin_list_init(&plugins);
    cmd_list_init(&cmds);
    symbol_table_init(&symbols);
    db_parser_init(&p);
    memset(&src, 0, sizeof(src));
    ast_init(&ast);
    ast_mutex = mutex_create();

    struct plugin_info* plugin = plugin_list_emplace(&plugins);
    plugin_info_init(plugin);
    utf8_set_cstr(&plugin->name, "test");
    ospath_set_cstr(&plugin->filepath, "test");
}

DBParserHelper::~DBParserHelper()
{
    if (ast)
    {
#if defined(ODBCOMPILER_DOT_EXPORT)
        const testing::TestInfo* info
            = testing::UnitTest::GetInstance()->current_test_info();
        std::string filename = std::string("ast/") + info->test_suite_name()
                               + "__" + info->name() + ".dot";
        std::filesystem::create_directory("ast");
        ast_export_dot(
            ast, cstr_ospathc(filename.c_str()), src.text.data, &cmds);
#endif
    }

    mutex_destroy(ast_mutex);

    struct plugin_info* plugin;
    vec_for_each(plugins, plugin)
    {
        ospath_deinit(plugin->filepath);
        utf8_deinit(plugin->name);
    }

    ast_deinit(ast);
    db_parser_deinit(&p);
    if (src.text.data)
        db_source_close(&src);
    symbol_table_deinit(symbols);
    cmd_list_deinit(&cmds);
    plugin_list_deinit(plugins);
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
DBParserHelper::semantic(const struct semantic_check* check)
{
    int         result;
    struct utf8 filename = empty_utf8();

    utf8_set_cstr(&filename, "test");
    result = semantic_check_run(
        check,
        &ast,
        1,
        0,
        &ast_mutex,
        &filename,
        &src,
        plugins,
        &cmds,
        symbols);
    utf8_deinit(filename);
    return result;
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
