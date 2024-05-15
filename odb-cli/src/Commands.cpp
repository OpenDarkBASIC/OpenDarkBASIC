#include "odb-cli/Commands.hpp"
#include "odb-cli/SDK.hpp"

extern "C" {
#include "odb-compiler/sdk/cmd_list.h"
#include "odb-compiler/sdk/plugin_list.h"
#include "odb-sdk/log.h"
}

static plugin_list plugins;
static cmd_list    commands;

void
initCommands(void)
{
    plugin_list_init(&plugins);
    cmd_list_init(&commands);
}
void
deinitCommands(void)
{
    struct plugin_info* plugin;

    cmd_list_deinit(&commands);
    vec_for_each(plugins, plugin)
    {
        plugin_info_deinit(plugin);
    }
    plugin_list_deinit(&plugins);
}

// ----------------------------------------------------------------------------
bool
loadCommands(const std::vector<std::string>& args)
{
    log_sdk_progress(0, 0, "Searching for plugins...\n");
    if (plugin_list_populate(&plugins, getSDKType(), getSDKRootDir(), NULL)
        != 0)
        return false;

    log_sdk_progress(0, 0, "Loading commands...\n");
    if (cmd_list_load_from_plugins(&commands, getSDKType(), plugins) != 0)
        return false;
    log_sdk_info(
        "Loaded %d commands from %d plugins\n",
        cmd_list_count(&commands),
        vec_count(plugins));

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
        enum cmd_param_type ret_type = *vec_get(commands.return_types, i);
        printf("%c ", ret_type);
        printf("%s", utf8_list_cstr(&commands.db_identifiers, i));
        printf("(");
        struct param_types_list* params = vec_get(commands.param_types, i);
        struct cmd_param*        param;
        vec_for_each(*params, param)
        {
            printf("%c", param->type);
            if (param->direction == CMD_PARAM_OUT)
                printf("*");
        }
        printf(")\n");
    }
    log_sdk_info(
        "Wrote %d commands to stdout [--dump-commands]\n",
        cmd_list_count(&commands));

    return true;
}

struct cmd_list*
getCommandList(void)
{
    return &commands;
}
