#include "odb-cli/Codegen.hpp"
#include "odb-cli/Commands.hpp"
#include "odb-cli/SDK.hpp"

extern "C" {
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/sdk/plugin_list.h"
#include "odb-compiler/sdk/type.h"
#include "odb-util/log.h"
}

static plugin_list* plugins;
static cmd_list     commands;
static ospath_list  extra_plugins;

void
initCommands(void)
{
    plugin_list_init(&plugins);
    cmd_list_init(&commands);
    ospath_list_init(&extra_plugins);
}
void
deinitCommands(void)
{
    struct plugin_info* plugin;

    ospath_list_deinit(extra_plugins);
    cmd_list_deinit(&commands);
    vec_for_each(plugins, plugin)
    {
        plugin_info_deinit(plugin);
    }
    plugin_list_deinit(plugins);
}

// ----------------------------------------------------------------------------
bool
loadCommands(const std::vector<std::string>& args)
{
    log_cmd_progress(0, 0, "Searching for plugins...\n");
    if (plugin_list_populate(
            &plugins,
            getSDKType(),
            getTargetPlatform(),
            getSDKRootDir(),
            &extra_plugins)
        != 0)
        return false;

    log_cmd_progress(0, 0, "Loading commands...\n");
    if (cmd_list_load_from_plugins(
            &commands,
            plugins,
            getSDKType(),
            getTargetArch(),
            getTargetPlatform())
        != 0)
        return false;
    log_cmd_info(
        "Loaded %d commands from %d plugins\n",
        cmd_list_count(&commands),
        plugins->count);

    /*
    std::unique_ptr<odb::cmd::CommandLoader> loader;
    switch (getSDKType())
    {
        case odb::SDKType::ODB :
            loader = std::make_unique<cmd::ODBCommandLoader>(getSDKRootDir(),
    getAdditionalPluginDirs()); break; case odb::SDKType::DarkBASIC : loader =
    std::make_unique<cmd::DBPCommandLoader>(getSDKRootDir(),
    getAdditionalPluginDirs()); break;
    }

    if (!loader->populateIndex(&cmdIndex_))
        return false;

    if (cmdIndex_.findConflicts())
        return false;

    Log::cmd(Log::INFO, "Loaded %d commands\n", cmdIndex_.commands().size());*/

    return true;
}

// ----------------------------------------------------------------------------
bool
dumpCommandsJSON(const std::vector<std::string>& args)
{
#if 0
    std::vector<Reference<cmd::Command>> commands = cmdIndex_.commands();
    std::sort(commands.begin(), commands.end(), [](const cmd::Command* a, const cmd::Command* b) { return a->dbSymbol() < b->dbSymbol(); });

    Log::data.print("{\n");
    for (const auto& command : commands)
    {
        Log::data.print("  \"%s\": {\n", command->dbSymbol().c_str());
        Log::data.print("    \"help\": \"%s\",\n", command->helpFile().c_str());
        Log::data.print("    \"overloads\": [\n");
        for (auto overload = command->overloads.begin(); overload != command->overloads.end(); ++overload)
        {
            Log::data.print("      {\n");
            Log::data.print("        \"returnType\": \"%s\",\n", command->returnType.has_value() ? std::string{(char)command->returnType.value()}.c_str() : "void");
            Log::data.print("        \"args\": [");
            auto arg = overload->arglist.begin();
            if (arg != overload->arglist.end())
                Log::data.print("\"%s\"", (*arg++).name.c_str());
            while (arg != overload->arglist.end())
                Log::data.print(", \"%s\"", (*arg++).name.c_str());
            Log::data.print("]\n");
            Log::data.print("      }%s\n", overload + 1 != command->overloads.end() ? ", " : "");
        }
        Log::data.print("    ]\n");
        Log::data.print("  }%s\n", command + 1 != commands.end() ? "," : "");
    }
    Log::data.print("}\n");

    Log::cmdParser(Log::INFO, "Commands dumped\n");
#endif
    return true;
}

// ----------------------------------------------------------------------------
bool
dumpCommandsINI(const std::vector<std::string>& args)
{
#if 0
    auto commands = cmdIndex_.commandsAsList();
    std::sort(commands.begin(), commands.end(), [](const Command &a, const Command &b) { return a.name < b.name; });

    Log::data.print("[LINKS]\n");
    for (const auto& command : commands)
    {
        Log::data.print("%s=%s=", command.name.c_str(), command.helpFile.c_str());
        for (const auto &overload : command.overloads)
        {
            if (command.overloads.size() > 1)
                Log::data.print("[");
            if (command.returnType.has_value())
                Log::data.print("(");

            auto arg = overload.arglist.begin();
            if (arg != overload.arglist.end())
                Log::data.print("%s", (*arg++).name.c_str());
            else
                Log::data.print("*no parameters*");
            while (arg != overload.arglist.end())
                Log::data.print(", %s", (*arg++).name.c_str());

            if (command.returnType)
                Log::data.print(")");
            if (command.overloads.size() > 1)
                Log::data.print("]");
        }
        Log::data.print("\n");
    }

    Log::cmdParser(Log::INFO, "Commands dumped\n");
#endif
    return true;
}

// ----------------------------------------------------------------------------
bool
dumpCommandNames(const std::vector<std::string>& args)
{
    for (int i = 0; i != cmd_list_count(&commands); ++i)
    {
        enum type ret_type = commands.return_types->data[i];
        printf("%s ", type_to_db_name(ret_type));
        printf("%s", utf8_list_cstr(commands.db_cmd_names, i));
        printf("%s", ret_type == TYPE_VOID ? " " : "(");
        const struct cmd_param_types_list* param_types
            = commands.param_types->data[i];
        struct utf8_list*       param_names = commands.db_param_names->data[i];
        const struct cmd_param* param;
        int                     n;
        vec_enumerate(param_types, n, param)
        {
            if (n)
                printf(", ");
            printf("%s", utf8_list_cstr(param_names, n));
            if (param->direction == CMD_PARAM_OUT)
                printf("*");
            printf(" AS %s", type_to_db_name(param->type));
        }
        if (ret_type != TYPE_VOID)
            printf(")");
        printf(" -> %s", utf8_list_cstr(commands.c_symbols, i));
        printf(
            "  [%s]\n",
            utf8_cstr(plugins->data[commands.plugin_ids->data[i]].name));
    }
    log_cmd_info(
        "Wrote %d commands to stdout [--dump-commands]\n",
        cmd_list_count(&commands));

    return true;
}

const struct plugin_list*
getPluginList(void)
{
    return plugins;
}

const struct cmd_list*
getCommandList(void)
{
    return &commands;
}
