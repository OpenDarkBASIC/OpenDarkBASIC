#include "odb-compiler/tests/DBParserHelper.hpp"
#include <filesystem>

#include <gtest/gtest.h>

extern "C" {
#include "odb-compiler/ast/ast_export.h"
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/semantic/globals.h"
#include "odb-compiler/semantic/semantic.h"
#include "odb-compiler/semantic/type.h"
#include "odb-util/mutex.h"
#include "odb-util/utf8.h"

extern int         odbtests_ast;
extern const char* odbtests_ast_filename;
}

using namespace testing;

DBParserHelper::DBParserHelper()
{
    plugin_list_init(&plugins);
    udt_storage_init(&udts);
    cmd_list_init(&cmds);
    globals_init(&globals);
    db_parser_init(&p);
    ospath_list_init(&filenames);

    ospath_list_add_cstr(&filenames, "test");

    memset(&src, 0, sizeof(src));
    ast_init(&ast);
    ast_mutex = mutex_create();
    cmd_list_mutex = mutex_create();

    struct plugin_info* plugin = plugin_list_emplace(&plugins);
    plugin_info_init(plugin);
    utf8_set_cstr(&plugin->name, "test");
    ospath_set_cstr(&plugin->filepath, "test");
}

DBParserHelper::~DBParserHelper()
{
    writeAST();

    mutex_destroy(cmd_list_mutex);
    mutex_destroy(ast_mutex);
    ast_deinit(ast);

    struct plugin_info* plugin;
    vec_for_each(plugins, plugin)
    {
        ospath_deinit(plugin->filepath);
        utf8_deinit(plugin->name);
    }

    db_parser_deinit(&p);
    utf8_deinit(src);
    ospath_list_deinit(filenames);
    globals_deinit(globals);
    cmd_list_deinit(&cmds);
    udt_storage_deinit(&udts);
    plugin_list_deinit(plugins);
}

int
DBParserHelper::parse(const char* code)
{
    utf8_deinit(src);
    src = empty_utf8();
    utf8_set_cstr(&src, code);

    if (ast)
    {
        ast_deinit(ast);
        ast_init(&ast);
    }

    int result = db_parse(
        &p, &ast, ospath_list_get(filenames, 0), &src, &plugins, &cmds, &udts);
    if (result != 0)
        return result;

#if defined(ODBCOMPILER_AST_DUMP)
    const testing::TestInfo* info
        = testing::UnitTest::GetInstance()->current_test_info();
    std::string astfile = std::string("ast/") + info->test_suite_name() + "__"
                          + info->name() + ".ast";
    std::filesystem::create_directory("ast");
    ast_export(ast, cstr_ospathc(astfile.c_str()), utf8_view(src), &cmds);
#endif

    return globals_add_declarations_from_ast(
        &globals, &ast, 0, ospath_list_ospathc(filenames), &src);
}

int
DBParserHelper::semantic(const struct semantic_check* check)
{
    int result = semantic_check_run(
        check,
        &ast,
        1,
        0,
        &ast_mutex,
        ospath_list_ospathc(filenames),
        &src,
        plugins,
        &cmds,
        &udts,
        globals);
#if defined(ODBCOMPILER_AST_DUMP)
    const testing::TestInfo* info
        = testing::UnitTest::GetInstance()->current_test_info();
    std::string astfile = std::string("ast/") + info->test_suite_name() + "__"
                          + info->name() + ".ast";
    std::filesystem::create_directory("ast");
    ast_export(ast, cstr_ospathc(astfile.c_str()), utf8_view(src), &cmds);
#endif
    return result;
}

int
DBParserHelper::addCommand(const char* name)
{
    return addCommand(TYPE_VOID, name);
}
int
DBParserHelper::addCommand(enum primitive_type return_type, const char* name)
{
    return addCommand(return_type, name, {});
}
int
DBParserHelper::addCommand(union type return_type, const char* name)
{
    return addCommand(return_type, name, {});
}
int
DBParserHelper::addCommand(
    union type                        return_type,
    const char*                       name,
    std::initializer_list<union type> param_types)
{
    cmd_id cmd = cmd_list_add(
        &cmds, 0, return_type, cstr_utf8_view(name), empty_utf8_view());
    if (cmd < 0)
        return cmd;

    for (union type type : param_types)
        if (cmd_add_param(
                &cmds,
                cmd,
                type,
                CMD_PARAM_IN,
                type_name(type, udts.ast, udts.source.data))
            < 0)
        {
            cmd_list_erase(&cmds, cmd);
            return -1;
        }

    return cmd;
}
int
DBParserHelper::addCommand(
    enum primitive_type                        return_type,
    const char*                                name,
    std::initializer_list<enum primitive_type> param_types)
{
    cmd_id cmd = cmd_list_add(
        &cmds,
        0,
        primitive_type(return_type),
        cstr_utf8_view(name),
        empty_utf8_view());
    if (cmd < 0)
        return cmd;

    for (enum primitive_type type : param_types)
        if (cmd_add_param(
                &cmds,
                cmd,
                primitive_type(type),
                CMD_PARAM_IN,
                cstr_utf8_view(primitive_type_name(type)))
            < 0)
        {
            cmd_list_erase(&cmds, cmd);
            return -1;
        }

    return cmd;
}

void
DBParserHelper::writeAST()
{
    if (ast == nullptr || !odbtests_ast)
        return;

    if (odbtests_ast_filename == NULL)
        ast_export_fp(ast, stdout, utf8_view(src), &cmds);
    else
        ast_export(
            ast, cstr_ospathc(odbtests_ast_filename), utf8_view(src), &cmds);
}
